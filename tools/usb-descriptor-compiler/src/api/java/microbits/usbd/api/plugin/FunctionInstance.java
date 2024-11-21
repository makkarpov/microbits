package microbits.usbd.api.plugin;

public interface FunctionInstance {
    /** @return Function identifier (absolute position in the function list) */
    int identifier();

    /** @return User-specified function name */
    String name();
}
