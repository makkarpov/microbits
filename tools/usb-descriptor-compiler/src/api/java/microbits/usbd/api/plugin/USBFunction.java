package microbits.usbd.api.plugin;

import microbits.usbd.api.codegen.FunctionCodeGenerator;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.endpoint.EndpointDefinition;
import microbits.usbd.api.iface.InterfaceDefinition;
import microbits.usbd.api.target.LinkSpeed;

import javax.annotation.Nullable;
import java.util.List;

public interface USBFunction {
    /**
     * @return Implementation identifier string to be checked against runtime-provided implementation
     */
    String implementationId();

    /**
     * Return definitions for all data endpoints used by the function.
     *
     * <p>
     *     Return value of this method is used to allocate and configure hardware endpoint resources. For functions with
     *     multiple alternate settings this list should be the worst-case scenario (max packet sizes, all endpoints).
     * </p>
     *
     * @param speed Target link speed. Might be lower than maximum supported device speed.
     * @return List of endpoint definitions
     */
    List<EndpointDefinition> endpoints(LinkSpeed speed);

    /**
     * Return definitions for all interfaces used by the function.
     *
     * @param speed Target link speed
     * @return List of interface definitions
     */
    List<InterfaceDefinition> interfaces(LinkSpeed speed);

    /**
     * Return a list of all relevant configuration descriptors, starting from the first interface of the function
     *
     * @param speed Target link speed (might be lower than supported maximum speed)
     * @return List of required configuration descriptors
     */
    List<Descriptor> descriptors(LinkSpeed speed, DescriptorResources resources);

    /** @return Code generator instance to customize produced file */
    @Nullable
    default FunctionCodeGenerator codeGenerator() { return null; }
}
