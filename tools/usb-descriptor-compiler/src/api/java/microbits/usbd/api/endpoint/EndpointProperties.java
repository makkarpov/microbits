package microbits.usbd.api.endpoint;

/**
 * Endpoint properties are used to generate endpoint descriptors, possibly overriding some fields from underlying
 * endpoint definitions.
 */
public final class EndpointProperties {
    /**
     * Overridden packet size. If {@code null}, packet size from endpoint definition is used.
     */
    public final Integer packetSize;

    /** Synchronization type for isochronous endpoints. */
    public final EndpointSync sync;

    /** Usage type for isochronous endpoints. */
    public final EndpointUsage usage;

    /**
     * Number of transfers per microframe for high-speed isochronous and interrupt endpoints.
     */
    public final Integer transfersPerFrame;

    /**
     * Endpoint polling interval in milliseconds for isochronous and interrupt endpoints. This value will be
     * rounded up to the nearest representable value.
     */
    public final Float interval;

    @Override
    public String toString() {
        return "EndpointProperties{" +
                "packetSize=" + packetSize +
                ", sync=" + sync +
                ", usage=" + usage +
                ", transfersPerFrame=" + transfersPerFrame +
                ", interval=" + interval +
                '}';
    }

    /** @return Builder for construction of this class */
    public static EndpointPropertiesBuilder builder() { return new EndpointPropertiesBuilder(); }

    EndpointProperties(EndpointPropertiesBuilder builder) {
        this.packetSize = builder.packetSize;
        this.sync = builder.sync;
        this.usage = builder.usage;
        this.transfersPerFrame = builder.transfersPerFrame;
        this.interval = builder.interval;
    }
}
