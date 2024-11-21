package microbits.usbd.core.specification;

import java.util.Objects;

public class ControlPipeSpecification {
    /** Maximum length of control packet */
    public final int maxPacket;

    public ControlPipeSpecification(ParsedObject spec) {
        maxPacket = Objects.requireNonNullElse(spec.integerOpt("maxPacket"), 64);
    }
}
