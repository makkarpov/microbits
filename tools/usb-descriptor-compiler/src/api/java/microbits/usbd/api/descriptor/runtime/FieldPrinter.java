package microbits.usbd.api.descriptor.runtime;

import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.target.LinkSpeed;

public interface FieldPrinter {
    String printField(LinkSpeed speed, Descriptor descriptor, int value);
}
