package microbits.usbd.core.descriptor;

import microbits.usbd.api.target.LinkSpeed;

public record DescriptorContext(StringDescriptorContainer strings, LinkSpeed speed) {
}
