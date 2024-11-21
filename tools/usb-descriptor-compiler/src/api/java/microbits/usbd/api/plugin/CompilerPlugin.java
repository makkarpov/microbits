package microbits.usbd.api.plugin;

import microbits.usbd.api.util.SpecificationEntry;
import microbits.usbd.api.target.TargetProperties;

import java.util.Set;

public interface CompilerPlugin {
    /** @return Function types recognized by the plugin */
    default Set<String> functionTypes() {
        return Set.of();
    }

    /**
     * Create actual function implementation.
     *
     * @param type Type of the function, as written in the specification.
     *             Guaranteed to match one of the functions in plugin's {@link #functionTypes()} set.
     * @param spec Specification entry for the function, to extract plugin-specific parameters
     * @return Descriptor function implementation
     */
    default USBFunction createFunction(String type, SpecificationEntry spec, FunctionEnvironment env) {
        throw new UnsupportedOperationException("createFunction() is not implemented in default DescriptorPlugin");
    }

    /** @return Target names recognized by the plugin */
    default Set<String> targetNames() {
        return Set.of();
    }

    /**
     * Fetch the properties for the target.
     *
     * @param name Target name, as written in the specification file.
     *             Guaranteed to match one of the target in plugin's {@link #targetNames()} set.
     * @return Target properties
     */
    default TargetProperties lookupTarget(String name) {
        throw new UnsupportedOperationException("lookupTarget() is not implemented in default DescriptorPlugin");
    }
}
