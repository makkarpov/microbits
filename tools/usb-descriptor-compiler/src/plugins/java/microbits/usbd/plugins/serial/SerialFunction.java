package microbits.usbd.plugins.serial;

import microbits.usbd.api.codegen.ConfigProperty;
import microbits.usbd.api.codegen.FunctionCodeGenerator;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.InterfaceAssociationDescriptor;
import microbits.usbd.api.descriptor.InterfaceDescriptor;
import microbits.usbd.api.endpoint.EndpointDefinition;
import microbits.usbd.api.endpoint.EndpointProperties;
import microbits.usbd.api.iface.InterfaceDefinition;
import microbits.usbd.api.plugin.DescriptorResources;
import microbits.usbd.api.plugin.FunctionEnvironment;
import microbits.usbd.api.plugin.USBFunction;
import microbits.usbd.api.target.LinkSpeed;

import javax.annotation.Nullable;
import java.util.List;

public class SerialFunction implements USBFunction, FunctionCodeGenerator {
    private static final ConfigProperty SERIAL_PACKET =
            new ConfigProperty("UB_USBD_SERIAL_PACKET_LENGTH", ConfigProperty.Type.SIZE);

    private final FunctionEnvironment environment;
    private final int packetSize;
    private final int dataBuffers;

    public SerialFunction(FunctionEnvironment env, int packetSize, int dataBuffers) {
        this.environment = env;
        this.packetSize = packetSize;
        this.dataBuffers = dataBuffers;

        if (packetSize <= 0) {
            throw new IllegalArgumentException("Packet size must be at least one byte");
        }

        if (packetSize > env.speed().maxBulkPacket) {
            throw new IllegalArgumentException(String.format(
                    "Packet size %d is larger than maximum allowed for the link speed %s (%d)",
                    packetSize, env.speed(), env.speed().maxBulkPacket
            ));
        }
    }

    @Override
    public String implementationId() {
        return "microbits.cdc-acm.v1";
    }

    @Override
    public List<EndpointDefinition> endpoints(LinkSpeed speed) {
        return SerialUtils.createEndpoints(speed, packetSize, dataBuffers);
    }

    @Override
    public List<InterfaceDefinition> interfaces(LinkSpeed speed) {
        return SerialUtils.createInterfaces();
    }

    @Override
    public List<Descriptor> descriptors(LinkSpeed speed, DescriptorResources resources) {
        return List.of(
                InterfaceAssociationDescriptor.builder()
                        .bFirstInterface(resources.interfaceBase())
                        .bInterfaceCount(2)
                        .bFunctionClass(CLASS_CDC)
                        .bFunctionSubclass(SUBCLASS_CDC_ACM)
                        .bFunctionProtocol(0)
                        .build(),
                InterfaceDescriptor.builder()
                        .bInterfaceNumber(resources.interfaceNumber(SerialUtils.I_CONTROL))
                        .bAlternateSetting(0)
                        .bNumEndpoints(1)
                        .bInterfaceClass(CLASS_CDC)
                        .bInterfaceSubclass(SUBCLASS_CDC_ACM)
                        .bInterfaceProtocol(0)
                        .build(),
                CDCDescriptor.HeaderDescriptor.builder()
                        .bcdCDC(0x0111)
                        .build(),
                CDCDescriptor.CallManagementDescriptor.builder()
                        .bmCapabilities(0)
                        .bDataInterface(resources.interfaceNumber(SerialUtils.I_DATA))
                        .build(),
                CDCDescriptor.ACMDescriptor.builder()
                        .bmCapabilities(0)
                        .build(),
                CDCDescriptor.InterfaceUnionDescriptor.builder()
                        .bMasterInterface(resources.interfaceNumber(SerialUtils.I_CONTROL))
                        .bSlaveInterface(resources.interfaceNumber(SerialUtils.I_DATA))
                        .build(),
                resources.endpointDescriptor(SerialUtils.EP_NOTIFICATIONS, EndpointProperties.builder()
                        .interval(1)
                        .build()),
                InterfaceDescriptor.builder()
                        .bInterfaceNumber(resources.interfaceNumber(SerialUtils.I_DATA))
                        .bAlternateSetting(0)
                        .bNumEndpoints(2)
                        .bInterfaceClass(CLASS_CDC_DATA)
                        .bInterfaceSubclass(0)
                        .bInterfaceProtocol(0)
                        .build(),
                resources.endpointDescriptor(SerialUtils.EP_DATA_IN, null),
                resources.endpointDescriptor(SerialUtils.EP_DATA_OUT, null)
        );
    }

    @Nullable
    @Override
    public FunctionCodeGenerator codeGenerator() {
        return this;
    }

    @Override
    public List<String> includeFiles() {
        return List.of("ub/usbd/serial-config.hpp");
    }

    @Override
    public List<ConfigProperty.Check> configurationChecks() {
        return List.of(SERIAL_PACKET.createCheck(packetSize));
    }

    @Override
    public String toString() {
        return "SerialFunction";
    }

    private static final int CLASS_CDC = 0x02;
    private static final int CLASS_CDC_DATA = 0x0A;
    private static final int SUBCLASS_CDC_ACM = 0x02;
}
