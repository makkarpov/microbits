package microbits.usbd.core.state;

import lombok.AllArgsConstructor;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.InterfaceAssociationDescriptor;
import microbits.usbd.api.descriptor.InterfaceDescriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.endpoint.*;
import microbits.usbd.api.iface.InterfaceDefinition;
import microbits.usbd.api.plugin.DescriptorResources;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.core.DescriptorCompiler;
import microbits.usbd.core.descriptor.DescriptorSerializer;
import microbits.usbd.core.descriptor.StringDescriptorContainer;
import microbits.usbd.core.descriptor.structs.ConfigurationDescriptor;
import microbits.usbd.core.descriptor.structs.EndpointDescriptor;

import javax.annotation.Nullable;
import java.util.*;

/** Descriptor state for a given link speed */
public class DescriptorState {
    public final DescriptorCompiler compiler;
    public final LinkSpeed speed;

    public final EndpointAllocation endpointAllocation;

    public final InterfaceMapping interfaces;

    public final List<Descriptor> configDescriptor;

    public final boolean haveInterfaceAssociations;

    public DescriptorState(DescriptorCompiler compiler, LinkSpeed speed) {
        this.compiler = compiler;
        this.speed = speed;

        endpointAllocation = allocateEndpoints();
        interfaces = allocateInterfaces();
        configDescriptor = generateConfigDescriptors();

        haveInterfaceAssociations = configDescriptor.stream()
                .anyMatch(d -> d instanceof InterfaceAssociationDescriptor);
    }

    public boolean hasMultipleInterfaces() {
        return interfaces.numInterfaces() > 1;
    }

    public InterfaceDescriptor firstInterfaceDescriptor() {
        return (InterfaceDescriptor) configDescriptor.stream()
                .filter(x -> x instanceof InterfaceDescriptor)
                .findFirst()
                .orElseThrow(() -> new RuntimeException("No interface descriptor found"));
    }

    private EndpointAllocation allocateEndpoints() {
        EndpointAllocator alloc = new EndpointAllocator(compiler.target());

        for (FunctionState st: compiler.functions()) {
            List<EndpointDefinition> endpoints = st.impl.endpoints(speed);

            try {
                validateEndpoints(endpoints);
            } catch (IllegalArgumentException e) {
                throw new RuntimeException(String.format(
                        "Endpoint validation failed for function '%s', speed %s: %s",
                        st.name, speed, e.getMessage()
                ));
            }

            for (EndpointDefinition ep: endpoints) {
                alloc.addEndpoint(st, ep);
            }
        }

        return alloc.finish(compiler.spec.controlPipe, compiler.functions());
    }

    private InterfaceMapping allocateInterfaces() {
        int nextInterface = 0;

        List<InterfaceMapping.FnData> functions = new ArrayList<>();
        List<InterfaceMapping.FnIntf> physical = new ArrayList<>();

        for (FunctionState st: compiler.functions()) {
            List<InterfaceDefinition> itfs = st.impl.interfaces(speed);
            validateInterfaces(itfs);

            List<InterfaceDefinition> sorted = itfs.stream()
                    .sorted(Comparator.comparingInt(i -> i.logicalNumber))
                    .toList();

            functions.add(new InterfaceMapping.FnData(nextInterface, sorted));

            for (InterfaceDefinition itf: sorted) {
                physical.add(new InterfaceMapping.FnIntf(st, itf));
                nextInterface += 1;
            }
        }

        return new InterfaceMapping(List.copyOf(physical), List.copyOf(functions));
    }

    private void validateEndpoints(List<EndpointDefinition> endpoints) {
        Set<Integer> usedLogicalAddresses = new HashSet<>();

        for (EndpointDefinition ep: endpoints) {
            if (usedLogicalAddresses.contains(ep.logicalAddress)) {
                throw new IllegalArgumentException(String.format("Duplicate logical addresses: %d", ep.logicalAddress));
            }

            if (ep.type == EndpointType.CONTROL) {
                throw new IllegalArgumentException(String.format("Endpoint has CONTROL type: '%s'", ep.name ));
            }

            int maxPacket = speed.maxPacket(ep.type);
            if (ep.packetSize > maxPacket) {
                throw new IllegalArgumentException(String.format(
                        "Endpoint '%s' has too big packet size: %d > %d (type %s)",
                        ep.name, ep.packetSize, maxPacket, ep.type
                ));
            }

            usedLogicalAddresses.add(ep.logicalAddress);
        }
    }

    private void validateInterfaces(List<InterfaceDefinition> interfaces) {
        int maxLogicalNumber = interfaces.size() - 1;
        Set<Integer> usedLogicalNumbers = new HashSet<>();

        for (InterfaceDefinition defn: interfaces) {
            if (defn.logicalNumber < 0 || defn.logicalNumber > maxLogicalNumber) {
                throw new IllegalArgumentException(String.format("Logical number is out of range: %d",
                        defn.logicalNumber));
            }

            if (usedLogicalNumbers.contains(defn.logicalNumber)) {
                throw new IllegalArgumentException(String.format("Duplicate logical numbers: %d", defn.logicalNumber));
            }

            usedLogicalNumbers.add(defn.logicalNumber);
        }
    }

    private List<Descriptor> generateConfigDescriptors() {
        ArrayList<Descriptor> ret = new ArrayList<>();
        ret.add(0, null);

        for (FunctionState st: compiler.functions()) {
            ret.addAll(st.impl.descriptors(speed, new DescriptorResourcesImpl(st)));
        }

        int interfaceCount = (int) ret.stream()
                .filter(x -> x instanceof InterfaceDescriptor)
                .count();

        if (interfaceCount != interfaces.numInterfaces()) {
            throw new RuntimeException("Mismatched interface descriptor count");
        }

        int totalLength = ConfigurationDescriptor.LENGTH;
        for (int i = 1; i < ret.size(); i++) {
            if (ret.get(i) == null) {
                throw new RuntimeException("null descriptor encountered in the list");
            }

            totalLength += DescriptorSerializer.serializedLength(ret.get(i));
        }

        int maxPower = Math.min(Math.max(0, compiler.spec.device.maxPower / 2), 255);
        int attrs = ConfigurationDescriptor.ATTR_RESERVED_ONE;

        if (compiler.spec.device.selfPowered) {
            attrs |= ConfigurationDescriptor.ATTR_SELF_POWERED;
        }

        if (compiler.spec.device.remoteWakeup) {
            attrs |= ConfigurationDescriptor.ATTR_REMOTE_WAKEUP;
        }

        ret.set(0, ConfigurationDescriptor.builder()
                .wTotalLength(totalLength)
                .bNumInterfaces(interfaceCount)
                .bConfigurationValue(1)
                .bmAttributes(attrs)
                .bMaxPower(maxPower)
                .build());

        return List.copyOf(ret);
    }

    @AllArgsConstructor
    private class DescriptorResourcesImpl implements DescriptorResources {
        private final FunctionState st;

        @Override
        public StringReference stringDescriptor(String data) {
            return compiler.strings().putString(StringDescriptorContainer.Tag.OTHER, data);
        }

        @Override
        public int interfaceBase() {
            return interfaces.functions.get(st.id).interfaceBase();
        }

        @Override
        public int interfaceNumber(int logicalNumber) {
            InterfaceMapping.FnData fn = interfaces.functions.get(st.id);
            if (logicalNumber < 0 || logicalNumber >= fn.defs().size()) {
                throw new IllegalArgumentException("Logical interface number is out of range: " + logicalNumber);
            }

            return fn.interfaceBase() + logicalNumber;
        }

        @Override
        public Descriptor endpointDescriptor(int logicalAddress, @Nullable EndpointProperties properties) {
            if (properties == null) {
                properties = EndpointProperties.builder().build();
            }

            EndpointAllocation.PhysicalEndpoint ep = resolveEndpoint(logicalAddress);

            if (properties.packetSize != null && properties.packetSize > ep.maxPacket) {
                throw new IllegalArgumentException("packetSize is larger than definition's packet size: " +
                        properties.packetSize);
            }

            int extraAttributes = getExtraAttributes(properties, ep);
            int wMaxPacketSize = encodeMaxPacketSize(properties, ep);
            int interval = properties.interval != null ? encodeInterval(ep, properties.interval) : 1;

            return EndpointDescriptor.builder()
                    .bEndpointAddress(ep.number | ep.direction.directionBit)
                    .bmAttributes(ep.type.ordinal() | extraAttributes)
                    .wMaxPacketSize(wMaxPacketSize)
                    .bInterval(interval)
                    .build();
        }

        private int encodeMaxPacketSize(EndpointProperties properties, EndpointAllocation.PhysicalEndpoint ep) {
            boolean interruptOrIso = ep.type == EndpointType.INTERRUPT || ep.type == EndpointType.ISOCHRONOUS;
            if (properties.transfersPerFrame != null && (!interruptOrIso || speed != LinkSpeed.HIGH)) {
                throw new IllegalArgumentException("'transfersPerFrame' field is only allowed for INTERRUPT or " +
                        "ISOCHRONOUS endpoints and only for high speed links");
            }

            int packetSize = Objects.requireNonNullElse(properties.packetSize, ep.maxPacket);
            int transfersPerFrame = Objects.requireNonNullElse(properties.transfersPerFrame, 1);
            return packetSize | ((transfersPerFrame - 1) << 11);
        }

        private int getExtraAttributes(EndpointProperties properties, EndpointAllocation.PhysicalEndpoint ep) {
            int extraAttributes = 0;

            if (ep.type == EndpointType.ISOCHRONOUS) {
                EndpointSync sync = Objects.requireNonNullElse(properties.sync, EndpointSync.NONE);
                EndpointUsage usage = Objects.requireNonNullElse(properties.usage, EndpointUsage.DATA);
                extraAttributes = (sync.ordinal() << 2) | (usage.ordinal() << 4);
            } else {
                if (properties.sync != null) {
                    throw new IllegalArgumentException("'sync' field is only allowed for ISOCHRONOUS endpoints");
                }

                if (properties.usage != null) {
                    throw new IllegalArgumentException("'usage' field is only allowed for ISOCHRONOUS endpoints");
                }
            }

            return extraAttributes;
        }

        private int encodeInterval(EndpointAllocation.PhysicalEndpoint ep, float interval) {
            int frames = Math.max(1, (int) Math.ceil(interval / speed.frameIntervalMs));

            if (ep.type == EndpointType.BULK || (ep.type == EndpointType.INTERRUPT && speed == LinkSpeed.FULL)) {
                return Math.min(frames, 255);
            }

            int log2 = (int) Math.ceil(Math.log(frames) / Math.log(2));
            return Math.min(log2 + 1, 16);
        }

        private EndpointAllocation.PhysicalEndpoint resolveEndpoint(int logicalAddress) {
            EndpointAllocation.Function epList = endpointAllocation.functions.get(st.id);
            if (logicalAddress < 0 || logicalAddress >= epList.endpoints.size()) {
                throw new IllegalArgumentException("Logical address is out of range: " + logicalAddress);
            }

            EndpointAllocation.FunctionEp fnEp = epList.endpoints.get(logicalAddress);
            if (!fnEp.mapped) {
                throw new IllegalArgumentException("Logical endpoint is not mapped: " + logicalAddress);
            }

            return endpointAllocation.endpoints(fnEp.direction).get(fnEp.number);
        }
    }
}
