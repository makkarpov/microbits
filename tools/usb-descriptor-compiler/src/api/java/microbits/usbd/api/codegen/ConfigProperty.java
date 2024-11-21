package microbits.usbd.api.codegen;

import lombok.AccessLevel;
import lombok.AllArgsConstructor;
import lombok.ToString;
import microbits.usbd.api.util.InternalApi;

@SuppressWarnings("ClassCanBeRecord")
@ToString
public final class ConfigProperty {
    /** Name of the property preprocessor definition */
    public final String name;

    /** Property type */
    public final Type type;

    /**
     * Create a new configuration property definition.
     *
     * @param name Name of the configuration property
     * @param type Type of the property
     */
    public ConfigProperty(String name, Type type) {
        this.name = name;
        this.type = type;
    }

    /**
     * Create a new check object, checking this property against a value.
     *
     * @param value Value to check
     * @return      Created check object
     */
    public Check createCheck(int value) {
        return new Check(this, value);
    }

    @AllArgsConstructor(access = AccessLevel.PROTECTED)
    @InternalApi
    public static class Check {
        public final ConfigProperty property;
        public final int value;
    }

    public enum Type {
        /**
         * Property is a flag with allowed values of {@code 0} and {@code 1}.
         *
         * <p>Configuration checks are interpreted as the exact value requirement - flag should be set or clear.
         * Multiple conflicting checks raise compile-time error.</p>
         *
         * <p>Configuration suggestions suggest exact checked value.</p>
         */
        FLAG,

        /**
         * Property is an integer value with arbitrary semantic.
         *
         * <p>Configuration checks are interpreted as the exact value requirement. Multiple conflicting checks raise
         * compile-time error.</p>
         *
         * <p>Configuration suggestions suggest exact checked value.</p>
         */
        VALUE,

        /**
         * Property is an integer value which specifies maximum size for some statically allocated resource.
         *
         * <p>Configuration checks are interpreted as the minimum required size. Multiple checks are combined into
         * a single check with maximum of requested sizes.</p>
         *
         * <p>Configuration suggestions suggest maximum of checked values.</p>
         */
        SIZE
    }
}
