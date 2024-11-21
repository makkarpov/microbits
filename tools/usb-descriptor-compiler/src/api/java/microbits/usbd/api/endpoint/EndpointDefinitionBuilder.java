package microbits.usbd.api.endpoint;

import java.util.Objects;

public final class EndpointDefinitionBuilder {
    String name;
    EndpointType type;
    EndpointDirection direction;
    int logicalAddress = -1;
    int packetSize;
    int numBuffers = 1;

    /**
     * Set a logical name for the endpoint. Names should be unique within a function.
     *
     * <p><strong>This call is required to successfully construct an endpoint definition.</strong></p>
     *
     * @param name Endpoint name
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder name(String name) {
        this.name = Objects.requireNonNull(name);
        return this;
    }

    /**
     * Set a type for the endpoint.
     *
     * <p><strong>This call is required to successfully construct an endpoint definition.</strong></p>
     *
     * @param type Endpoint type
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder type(EndpointType type) {
        this.type = Objects.requireNonNull(type);
        return this;
    }

    /**
     * Set a direction for the endpoint.
     *
     * <p><strong>This call is required to successfully construct an endpoint definition.</strong></p>
     *
     * @param direction Endpoint direction
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder direction(EndpointDirection direction) {
        this.direction = Objects.requireNonNull(direction);
        return this;
    }

    /**
     * Set logical address (one which will be used in C++ code) for the endpoint. Logical addresses use flat addressing
     * scheme where each endpoint is assigned a sequential address starting from zero. No direction bit is used, both IN
     * and OUT endpoints share same address space. Logical addresses must be unique within a function.
     *
     * <p><strong>This call is required to successfully construct an endpoint definition.</strong></p>
     *
     * @param address Endpoint logical address
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder logicalAddress(int address) {
        this.logicalAddress = address;
        return this;
    }

    /**
     * Set maximum packet size for the endpoint. For endpoint having multiple alternate setting, worst-case packet size
     * must be used here.
     *
     * <p><strong>This call is required to successfully construct an endpoint definition.</strong></p>
     *
     * @param packetSize Packet size in bytes
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder packetSize(int packetSize) {
        this.packetSize = packetSize;
        return this;
    }

    /**
     * Set number of packet buffers (each of packet size) allocated for this endpoint.
     *
     * <p>Single buffer is allocated by default.</p>
     *
     * @param numBuffers Number of packet buffers
     * @return Builder for chained calls
     */
    public EndpointDefinitionBuilder numBuffers(int numBuffers) {
        this.numBuffers = numBuffers;
        return this;
    }

    public EndpointDefinition build() {
        if (name == null) {
            throw new IllegalArgumentException("'name' is not set");
        }

        if (type == null) {
            throw new IllegalArgumentException("'type' is not set");
        }

        if (direction == null) {
            throw new IllegalArgumentException("'direction' is not set");
        }

        if (logicalAddress < 0) {
            throw new IllegalArgumentException("'logicalAddress' is out of range: " + logicalAddress);
        }

        if (packetSize <= 0) {
            throw new IllegalArgumentException("'packetSize' is out of bounds: " + packetSize);
        }

        if (numBuffers <= 0) {
            throw new IllegalArgumentException("'numBuffers' is out of bounds: " + numBuffers);
        }

        return new EndpointDefinition(this);
    }
}
