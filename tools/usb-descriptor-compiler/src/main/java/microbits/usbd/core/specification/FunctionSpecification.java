package microbits.usbd.core.specification;

import javax.annotation.Nullable;

public class FunctionSpecification {
    /** Function type string */
    public final String functionType;

    /** User-defined function name for reference */
    @Nullable
    public final String name;

    /** Raw data object to consume in specific function implementations */
    @Nullable
    public final ParsedObject config;

    FunctionSpecification(ParsedObject data) {
        functionType = data.string("type");
        name = data.stringOpt("name");

        config = data.objectOpt("config");
    }

    @Override
    public String toString() {
        return "FunctionSpecification{" +
                "functionType='" + functionType + '\'' +
                ", name='" + name + '\'' +
                ", config=" + config +
                '}';
    }
}
