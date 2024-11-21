package microbits.usbd.api.endpoint;

import java.util.Objects;

public final class EndpointPropertiesBuilder {
    Integer packetSize;
    EndpointSync sync;
    EndpointUsage usage;
    Integer transfersPerFrame;
    Float interval;

    /**
     * Override endpoint's maximum packet size. Original packet size still acts as an upper limit to this parameter.
     *
     * <p>This option is useful to create multiple alternate settigns with increasingly larger packet sizes, while
     * hardware is configured for the worst case.</p>
     *
     * @param packetSize New packet size for the endpoint
     * @return Builder for chained calls
     */
    public EndpointPropertiesBuilder packetSize(int packetSize) {
        this.packetSize = packetSize;
        return this;
    }

    /**
     * Set isochoronous endpoint synchronization type. Default is {@link EndpointSync#NONE}.
     *
     * <p>Setting this field for any other endpoint type will result in exception during descriptor generation.</p>
     *
     * @param sync Endpoint synchronization type
     * @return Builder for chained calls
     */
    public EndpointPropertiesBuilder sync(EndpointSync sync) {
        this.sync = Objects.requireNonNull(sync);
        return this;
    }

    /**
     * Set isochronous endpoint usage type. Default is {@link EndpointUsage#DATA}.
     *
     * <p>Setting this field for any other endpoint type will result in exception during descriptor generation.</p>
     *
     * @param usage Endpoint usage type
     * @return Builder for chained calls
     */
    public EndpointPropertiesBuilder usage(EndpointUsage usage) {
        this.usage = Objects.requireNonNull(usage);
        return this;
    }

    /**
     * Set number of transfers per microframe for high-speed isochronous and interrupt endpoints. Allowed range is from
     * 1 to 3 transfers per microframe.
     *
     * <p>Setting this field for full-speed endpoints or for any other endpoint type will result in exception during
     * descriptor generation.</p>
     *
     * @param transfersPerFrame Number of transfers per microframe
     * @return Builder for chained calls
     */
    public EndpointPropertiesBuilder transfersPerFrame(int transfersPerFrame) {
        this.transfersPerFrame = transfersPerFrame;
        return this;
    }

    /**
     * Set endpoint polling interval in milliseconds. Zero value is interpreted as interval of one frame.
     *
     * <p>This value is always specified in milliseconds and is transformed to the USB wire representation by the
     * compiler. If specified value is not exactly representable in USB protocol, it is rounded up to nearest
     * representable value.</p>
     *
     * @param interval Endpoint polling interval
     * @return Builder for chained calls
     */
    public EndpointPropertiesBuilder interval(float interval) {
        this.interval = interval;
        return this;
    }

    /** Build the {@code EndpointProperties} object */
    public EndpointProperties build() {
        if (packetSize != null && packetSize <= 0) {
            throw new IllegalArgumentException("'packetSize' is out of range: " + packetSize);
        }

        if (transfersPerFrame != null && (transfersPerFrame < 1 || transfersPerFrame > 3)) {
            throw new IllegalArgumentException("'transfersPerFrame' is out of range: " + transfersPerFrame);
        }

        if (interval != null && interval < 0) {
            throw new IllegalArgumentException("'interval' is out of range: " + interval);
        }

        return new EndpointProperties(this);
    }
}