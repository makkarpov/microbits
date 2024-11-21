package microbits.usbd.api.target;

import microbits.usbd.api.codegen.TargetCodeGenerator;

import javax.annotation.Nullable;
import java.util.Set;
import java.util.function.Supplier;

/**
 *
 */
public final class TargetProperties {
    /** Target name */
    public final String name;

    /** Maximum number of IN endpoints supported by the target */
    public final int inEndpoints;

    /** Maximum number of OUT endpoints supported by the target */
    public final int outEndpoints;

    /** Target feature flags */
    public final Set<TargetFeature> features;

    /** Supported link speeds */
    public final Set<LinkSpeed> speeds;

    /** Supplier for target-specific code generator */
    @Nullable
    public final Supplier<TargetCodeGenerator> codeGenerator;

    /** @return Specification builder */
    public static TargetPropertiesBuilder builder() {
        return new TargetPropertiesBuilder();
    }

    // package-private constructor
    TargetProperties(TargetPropertiesBuilder builder) {
        name = builder.name;
        inEndpoints = builder.inEndpoints;
        outEndpoints = builder.outEndpoints;
        features = builder.features;
        speeds = builder.speeds;
        codeGenerator = builder.codeGenerator;
    }

    @Override
    public String toString() {
        return "TargetProperties{" +
                "inEndpoints=" + inEndpoints +
                ", outEndpoints=" + outEndpoints +
                ", features=" + features +
                ", speeds=" + speeds +
                ", codeGenerator=" + (codeGenerator != null ? "<present>" : "null") +
                '}';
    }
}
