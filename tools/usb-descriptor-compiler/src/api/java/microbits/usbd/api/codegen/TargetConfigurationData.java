package microbits.usbd.api.codegen;

import microbits.usbd.api.endpoint.EndpointAllocation;
import microbits.usbd.api.target.LinkSpeed;

import java.util.List;

public interface TargetConfigurationData {
    /** @return Enabled link speeds */
    List<LinkSpeed> speeds();

    /** @return Computed endpoint allocation for a speed */
    EndpointAllocation endpoints(LinkSpeed speed);
}
