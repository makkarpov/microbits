package microbits.usbd.api.descriptor.runtime;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface DescriptorField {
    /** Field name, if differs from Java name */
    String name() default "";

    /** Type of the field */
    FieldType value();

    Class<? extends FieldPrinter> printer() default FieldPrinter.class;
}
