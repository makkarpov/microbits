package microbits.usbd.api.target;

import microbits.usbd.api.codegen.TargetCodeGenerator;

import java.util.Objects;
import java.util.Set;
import java.util.function.Supplier;

public final class TargetPropertiesBuilder {
    String name;
    int inEndpoints = 0;
    int outEndpoints = 0;
    Set<TargetFeature> features = Set.of();
    Set<LinkSpeed> speeds = Set.of(LinkSpeed.FULL);
    Supplier<TargetCodeGenerator> codeGenerator = null;

    /**
     * Set a target's name.
     *
     * <p><strong>This call is required to successfully construct a target specification.</strong></p>
     *
     * @param name Name of the target
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder name(String name) {
        this.name = Objects.requireNonNull(name);
        return this;
    }

    /**
     * Set number of IN endpoints supported by target hardware.
     *
     * <p><strong>This call is required to successfully construct a target specification.</strong></p>
     *
     * @param inEndpoints Number of supported endpoints
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder inEndpoints(int inEndpoints) {
        this.inEndpoints = inEndpoints;
        return this;
    }

    /**
     * Set number of OUT endpoints supported by target hardware.
     *
     * <p><strong>This call is required to successfully construct a target specification.</strong></p>
     *
     * @param outEndpoints Number of supported endpoints
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder outEndpoints(int outEndpoints) {
        this.outEndpoints = outEndpoints;
        return this;
    }

    /**
     * Set flags representing target hardware endpoint assignment constraints.
     * Empty set of flags is assumed if this method is not called.
     *
     * @param targetFeatures Set of endpoint flags
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder targetFeatures(Set<TargetFeature> targetFeatures) {
        this.features = Set.copyOf(targetFeatures);
        return this;
    }

    /**
     * Set flags representing target hardware endpoint assignment constraints.
     * Empty set of flags is assumed if this method is not called.
     *
     * @param targetFeatures Set of endpoint flags
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder targetFeatures(TargetFeature... targetFeatures) {
        this.features = Set.of(targetFeatures);
        return this;
    }

    /**
     * Set hardware supported speeds for the device. Full speed (12 Mbps) is assumed by default.
     *
     * @param speeds Set of hardware supported speeds
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder speeds(Set<LinkSpeed> speeds) {
        this.speeds = Set.copyOf(speeds);
        return this;
    }

    /**
     * Set hardware supported speeds for the device. Full speed (12 Mbps) is assumed by default.
     *
     * @param speed Set of hardware supported speeds
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder speeds(LinkSpeed... speed) {
        this.speeds = Set.of(speed);
        return this;
    }

    /**
     * Set code generator implementation for the device. By default, {@code GenericTargetData} structure will be
     * generated.
     *
     * @param codeGenerator Code generator implementation
     * @return Builder for chained calls
     */
    public TargetPropertiesBuilder codeGenerator(Supplier<TargetCodeGenerator> codeGenerator) {
        this.codeGenerator = Objects.requireNonNull(codeGenerator);
        return this;
    }

    public TargetProperties build() {
        if (name == null) {
            throw new IllegalArgumentException("'name' must be defined");
        }

        if (inEndpoints <= 0 || inEndpoints > MAX_ENDPOINTS) {
            throw new IllegalArgumentException("'inEndpoints' is out of range: " + inEndpoints);
        }

        if (outEndpoints <= 0 || outEndpoints > MAX_ENDPOINTS) {
            throw new IllegalArgumentException("'outEndpoints' is out of range: " + outEndpoints);
        }

        if (speeds.isEmpty()) {
            throw new IllegalArgumentException("'speeds' must contain at least one link speed");
        }

        return new TargetProperties(this);
    }

    private static final int MAX_ENDPOINTS = 16;
}