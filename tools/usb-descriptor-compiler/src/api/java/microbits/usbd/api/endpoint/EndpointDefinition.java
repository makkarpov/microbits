package microbits.usbd.api.endpoint;

public final class EndpointDefinition {
    /** Per-function unique name of the endpoint */
    public final String name;

    /** Endpoint type */
    public final EndpointType type;

    /** Endpoint direction */
    public final EndpointDirection direction;

    /** Logical endpoint address visible to function implementation. */
    public final int logicalAddress;

    /** Maximum packet size */
    public final int packetSize;

    /** Number of packet buffers */
    public final int numBuffers;

    @Override
    public String toString() {
        return "EndpointDefinition{" +
                "name='" + name + '\'' +
                ", type=" + type +
                ", direction=" + direction +
                ", logicalAddress=" + logicalAddress +
                ", packetSize=" + packetSize +
                ", numBuffers=" + numBuffers +
                '}';
    }

    public static EndpointDefinitionBuilder builder() {
        return new EndpointDefinitionBuilder();
    }

    EndpointDefinition(EndpointDefinitionBuilder builder) {
        name = builder.name;
        type = builder.type;
        direction = builder.direction;
        logicalAddress = builder.logicalAddress;
        packetSize = builder.packetSize;
        numBuffers = builder.numBuffers;
    }
}
