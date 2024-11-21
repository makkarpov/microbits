package microbits.usbd.core;

import microbits.usbd.api.descriptor.InterfaceDescriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.plugin.CompilerPlugin;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.api.target.TargetProperties;
import microbits.usbd.core.codegen.CodePrintStream;
import microbits.usbd.core.codegen.DescriptorCodeGenerator;
import microbits.usbd.core.descriptor.StringDescriptorContainer;
import microbits.usbd.core.descriptor.structs.DeviceDescriptor;
import microbits.usbd.core.specification.FunctionSpecification;
import microbits.usbd.core.specification.SpecificationFile;
import microbits.usbd.core.state.DescriptorState;
import microbits.usbd.core.state.FunctionState;
import microbits.usbd.core.util.CompilerOptions;

import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Path;
import java.util.*;
import java.util.stream.Collectors;

public class DescriptorCompiler {
    public final CompilerOptions options;

    public final SpecificationFile spec;

    private List<CompilerPlugin> plugins;

    private TargetProperties target;

    private List<FunctionState> functions;

    private LinkSpeed speed;

    private StringDescriptorContainer strings;

    private Map<LinkSpeed, DescriptorState> descriptors;

    private DeviceDescriptor deviceDescriptor;

    private DescriptorCompiler(CompilerOptions options) throws Exception {
        this.options = options;

        try {
            this.spec = SpecificationFile.load(options.specificationFile);
        } catch (Exception e) {
            System.err.println("Failed to parse specification file:");
            System.err.println(e.getMessage());
            System.exit(1);

            throw e;
        }
    }

    public TargetProperties target() { return target; }

    public LinkSpeed speed() {
        return speed;
    }

    public List<FunctionState> functions() {
        return functions;
    }

    public Map<LinkSpeed, DescriptorState> descriptors() {
        return descriptors;
    }

    public StringDescriptorContainer strings() {
        return strings;
    }

    public DeviceDescriptor deviceDescriptor() { return deviceDescriptor; }

    private void loadPlugins() {
        URL[] pluginUrls = options.pluginFiles.stream().map(DescriptorCompiler::fileToUrl).toArray(URL[]::new);
        URLClassLoader urlLoader = new URLClassLoader("plugin", pluginUrls, getClass().getClassLoader());

        ArrayList<CompilerPlugin> ret = new ArrayList<>();
        for (CompilerPlugin pl: ServiceLoader.load(CompilerPlugin.class, urlLoader)) {
            ret.add(pl);
        }

        plugins = List.of(ret.toArray(CompilerPlugin[]::new));
    }

    private void lookupTarget() {
        for (CompilerPlugin pl: plugins) {
            if (!pl.targetNames().contains(spec.target)) {
                continue;
            }

            target = pl.lookupTarget(spec.target);

            if (target == null) {
                throw new RuntimeException(String.format("lookupTarget('%s') on %s returned null result",
                        spec.target, pl));
            }

            return;
        }

        throw new RuntimeException(String.format("Failed to resolve target '%s'", spec.target));
    }

    private void resolveFunctions() {
        Map<String, Integer> nameCounters = new HashMap<>();
        Set<String> usedNames = new HashSet<>();
        functions = new ArrayList<>(spec.functions.size());

        for (int i = 0; i < spec.functions.size(); i++) {
            FunctionSpecification fn = spec.functions.get(i);

            String name = fn.name;
            if (name == null) {
                int counter = nameCounters.getOrDefault(fn.functionType, 0);

                do {
                    name = fn.functionType + counter;
                    counter++;
                } while (usedNames.contains(name));

                usedNames.add(name);
                nameCounters.put(fn.functionType, counter);
            }

            FunctionState st = new FunctionState(i, name, this);

            for (CompilerPlugin pl: plugins) {
                if (!pl.functionTypes().contains(fn.functionType)) {
                    continue;
                }

                st.impl = pl.createFunction(fn.functionType, fn.config, st.env);
                if (st.impl == null) {
                    throw new RuntimeException(String.format(
                            "createFunction('%s', ...) on %s returned null result",
                            fn.functionType, pl
                    ));
                }
            }

            if (st.impl == null) {
                throw new RuntimeException(String.format(
                        "Failed to resolve function type '%s'",
                        fn.functionType
                ));
            }

            functions.add(st);
        }

        functions = List.copyOf(functions);
    }

    private void createDeviceDescriptor() {
        boolean haveIADs = descriptors.values().stream().anyMatch(d -> d.haveInterfaceAssociations);
        boolean haveMultipleItf = descriptors.values().stream().anyMatch(DescriptorState::hasMultipleInterfaces);

        DeviceDescriptor.DeviceDescriptorBuilder builder = DeviceDescriptor.builder();

        if (haveIADs) {
            // IAD device class 0xEF - 0x02 - 0x01
            builder.bDeviceClass(0xEF)
                    .bDeviceSubClass(0x02)
                    .bDeviceProtocol(0x01);
        } else if (!haveMultipleItf) {
            InterfaceDescriptor itf = descriptors.values().iterator().next().firstInterfaceDescriptor();
            builder.bDeviceClass(itf.bInterfaceClass)
                    .bDeviceSubClass(itf.bInterfaceSubclass)
                    .bDeviceProtocol(itf.bInterfaceProtocol);
        }

        StringReference manufacturer = spec.device.manufacturer != null
                ? strings.putString(StringDescriptorContainer.Tag.MANUFACTURER, spec.device.manufacturer)
                : null;

        StringReference product = spec.device.product != null
                ? strings.putString(StringDescriptorContainer.Tag.PRODUCT, spec.device.product)
                : null;

        StringReference serial = spec.device.runtimeSerial
                ? strings.putPlaceholder(StringDescriptorContainer.Tag.SERIAL)
                : spec.device.serial != null
                ? strings.putString(StringDescriptorContainer.Tag.SERIAL, spec.device.serial)
                : null;

        deviceDescriptor = builder
                .bcdUSB(0x0200)
                .bMaxPacketSize0(spec.controlPipe.maxPacket)
                .idVendor(spec.device.vendorId)
                .idProduct(spec.device.productId)
                .bcdDevice(0x0100)
                .iManufacturer(manufacturer)
                .iProduct(product)
                .iSerialNumber(serial)
                .bNumConfigurations(1)
                .build();
    }

    public void run() throws Exception {
        loadPlugins();
        lookupTarget();

        speed = Objects.requireNonNullElse(spec.speed, LinkSpeed.FULL);
        if (!target.speeds.contains(speed)) {
            throw new IllegalArgumentException(String.format(
                    "Requested link speed %s is not supported by the target (%s). Supported speeds: %s",
                    speed, target.name, target.speeds.stream().map(Enum::name).collect(Collectors.joining(", "))
            ));
        }

        resolveFunctions();
        strings = new StringDescriptorContainer();

        descriptors = new HashMap<>();
        for (int i = 0; i <= speed.ordinal(); i++) {
            LinkSpeed spd = LinkSpeed.values()[i];
            descriptors.put(spd, new DescriptorState(this, spd));
        }

        descriptors = Map.copyOf(descriptors);

        createDeviceDescriptor();
        strings.assignIndices();

        try (CodePrintStream output = new CodePrintStream(options.outputFile)) {
            new DescriptorCodeGenerator(this, output).run();
        }
    }

    public static void main(String[] args) throws Exception {
        CompilerOptions options = new CompilerOptions(args);
        new DescriptorCompiler(options).run();
    }

    private static URL fileToUrl(Path f) {
        try {
            return f.toUri().toURL();
        } catch (MalformedURLException e) {
            throw new RuntimeException("Failed to convert file to URL: " + f, e);
        }
    }
}
