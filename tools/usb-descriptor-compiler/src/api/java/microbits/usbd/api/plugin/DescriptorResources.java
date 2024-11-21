package microbits.usbd.api.plugin;

import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.endpoint.EndpointProperties;

import javax.annotation.Nullable;

public interface DescriptorResources {
    /** @return String reference for a string descriptor with specified contents */
    StringReference stringDescriptor(String data);

    /** @return Base interface number for a function */
    int interfaceBase();

    /** @return Physical interface number assigned to given logical interface */
    int interfaceNumber(int logicalNumber);

    /** @return Endpoint descriptor for an endpoint with provided additional properties */
    Descriptor endpointDescriptor(int logicalAddress, @Nullable EndpointProperties properties);
}
