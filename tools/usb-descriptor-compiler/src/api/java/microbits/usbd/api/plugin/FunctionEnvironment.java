package microbits.usbd.api.plugin;

import microbits.usbd.api.target.LinkSpeed;

public interface FunctionEnvironment {
    /** @return Maximum possible link speed */
    LinkSpeed speed();
}
