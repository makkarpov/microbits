package microbits.usbd.api.descriptor.runtime;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface DescriptorClass {
    int type();

    String typeName();

    NextTypeField nextField() default @NextTypeField(name = "", type = FieldType.UINT8);

    @interface NextTypeField {
        String name();

        FieldType type();
    }
}
