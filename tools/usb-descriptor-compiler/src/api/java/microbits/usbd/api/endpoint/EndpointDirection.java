package microbits.usbd.api.endpoint;

public enum EndpointDirection {
    IN  (0x80),
    OUT (0x00);

    /** USB direction bit - {@code 0x80} for IN endpoints, {@code 0x00} for OUT endpoints */
    public final int directionBit;

    EndpointDirection(int directionBit) {
        this.directionBit = directionBit;
    }
}
