package microbits.usbd.plugins.serial;

import microbits.usbd.api.endpoint.EndpointDefinition;
import microbits.usbd.api.endpoint.EndpointDirection;
import microbits.usbd.api.endpoint.EndpointType;
import microbits.usbd.api.iface.InterfaceDefinition;
import microbits.usbd.api.target.LinkSpeed;

import java.util.List;

public class SerialUtils {
    public static final int EP_NOTIFICATIONS    = 0;
    public static final int EP_DATA_IN          = 1;
    public static final int EP_DATA_OUT         = 2;

    public static final int I_CONTROL           = 0;
    public static final int I_DATA              = 1;

    public static List<EndpointDefinition> createEndpoints(LinkSpeed speed, int maxPacketSize, int dataBuffers) {
        return List.of(
                EndpointDefinition.builder()
                        .name("notifications")
                        .type(EndpointType.INTERRUPT)
                        .direction(EndpointDirection.IN)
                        .logicalAddress(EP_NOTIFICATIONS)
                        .packetSize(10)
                        .build(),
                EndpointDefinition.builder()
                        .name("dataIn")
                        .type(EndpointType.BULK)
                        .direction(EndpointDirection.IN)
                        .logicalAddress(EP_DATA_IN)
                        .packetSize(Math.min(maxPacketSize, speed.maxBulkPacket))
                        .numBuffers(dataBuffers)
                        .build(),
                EndpointDefinition.builder()
                        .name("dataOut")
                        .type(EndpointType.BULK)
                        .direction(EndpointDirection.OUT)
                        .logicalAddress(EP_DATA_OUT)
                        .packetSize(Math.min(maxPacketSize, speed.maxBulkPacket))
                        .numBuffers(dataBuffers)
                        .build()
        );
    }

    public static List<InterfaceDefinition> createInterfaces() {
        return List.of(
                InterfaceDefinition.builder()
                        .name("control")
                        .logicalNumber(I_CONTROL)
                        .build(),
                InterfaceDefinition.builder()
                        .name("data")
                        .logicalNumber(I_DATA)
                        .build()
        );
    }
}
