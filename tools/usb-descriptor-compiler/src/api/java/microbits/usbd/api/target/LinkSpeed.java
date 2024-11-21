package microbits.usbd.api.target;

import microbits.usbd.api.endpoint.EndpointType;

public enum LinkSpeed {
    /** USB full speed (12 Mbps) */
    FULL(64,    1023,   1.0F),

    /** USB high speed (480 Mbps) */
    HIGH(512,   1024,   0.125F);

    /** Maximum packet size allowed per specification for BULK and INTERRUPT transfers */
    public final int maxBulkPacket;

    /** Maximum packet size allowed per specification for ISOCHRONOUS transfers */
    public final int maxIsochronousPacket;

    /** Frame or microframe interval in milliseconds */
    public final float frameIntervalMs;

    LinkSpeed(int maxBulkPacket, int maxIsochronousPacket, float frameIntervalMs) {
        this.maxBulkPacket = maxBulkPacket;
        this.maxIsochronousPacket = maxIsochronousPacket;
        this.frameIntervalMs = frameIntervalMs;
    }

    /** @return Maximum packet size for endpoint type */
    public int maxPacket(EndpointType type) {
        return switch (type) {
            case BULK, INTERRUPT -> maxBulkPacket;
            case ISOCHRONOUS -> maxIsochronousPacket;
            default -> throw new IllegalArgumentException("Unsupported endpoint type: " + type);
        };
    }
}
