package microbits.usbd.core.descriptor;

import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldPrinter;
import microbits.usbd.api.descriptor.runtime.FieldType;

import java.lang.reflect.Field;
import java.util.*;

public class DescriptorSerializer {
    private static final String LENGTH_FIELD        = "bLength";
    private static final String DESC_TYPE_FIELD     = "bDescriptorType";

    private static final int LENGTH_FIELD_LEN       = 1;

    public static SerializedDescriptor serialize(DescriptorContext context, Descriptor descriptor) {
        ClassMetadata classMetadata = metadataCache.computeIfAbsent(descriptor.getClass(),
                DescriptorSerializer::computeMetadata);

        LinkedList<SerializedField> fields = new LinkedList<>();

        for (TypeField tf: classMetadata.typeFields) {
            try {
                String prettyType = String.format("0x%02X\t%s", tf.value, tf.valueText);
                byte[] serialized = serializeType(tf.type, tf.value);
                fields.add(new SerializedField(tf.name, prettyType, serialized));
            } catch (Exception e) {
                throw new RuntimeException("Failed to serialize descriptor type of " + descriptor, e);
            }
        }

        for (FieldMetadata f: classMetadata.fields) {
            try {
                SerializedField ser = f.type == FieldType.UINT8_STR
                        ? serializeString(context, descriptor, f)
                        : serializeField(context, descriptor, f);

                fields.add(ser);
            } catch (Exception e) {
                throw new RuntimeException(String.format(
                        "Failed to serialize field '%s' of descriptor %s: %s",
                        f.name, descriptor, e.getMessage()
                ), e);
            }
        }

        int totalLength = LENGTH_FIELD_LEN; // length of bLength field itself
        for (SerializedField f: fields) {
            totalLength += f.serialized().length;
        }

        byte[] extraData = descriptor.serialize();
        if (extraData != null) {
            totalLength += extraData.length;
        }

        try {
            fields.addFirst(new SerializedField(LENGTH_FIELD, String.valueOf(totalLength), serializeU8(totalLength)));
        } catch (Exception e) {
            throw new RuntimeException(String.format(
                    "Failed to serialize field '%s' of descriptor %s: %s",
                    LENGTH_FIELD, descriptor, e.getMessage()
            ), e);
        }

        return new SerializedDescriptor(descriptor, List.copyOf(fields), extraData, descriptor.prettyPrint());
    }

    public static int serializedLength(Descriptor descriptor) {
        ClassMetadata classMetadata = metadataCache.computeIfAbsent(descriptor.getClass(),
                DescriptorSerializer::computeMetadata);

        int ret = LENGTH_FIELD_LEN;

        for (TypeField tf: classMetadata.typeFields) {
            ret += tf.type.length;
        }

        for (FieldMetadata f: classMetadata.fields) {
            ret += f.type.length;
        }

        byte[] extraData = descriptor.serialize();
        if (extraData != null) {
            ret += extraData.length;
        }

        return ret;
    }

    private static SerializedField serializeString(DescriptorContext context, Descriptor descriptor, FieldMetadata f) {
        StringReference ref;
        try {
            ref = (StringReference) f.javaField.get(descriptor);
        } catch (Exception e) {
            throw new RuntimeException("Failed to read field " + f.javaField.getName(), e);
        }

        int index = ref != null ? context.strings().getIndex(ref) : 0;
        String pretty = ref != null ? String.format("%d\t%s", index, ref.repr()) : "0";
        byte[] serialized = serializeU8(index);

        return new SerializedField(f.name, pretty, serialized);
    }

    private static SerializedField serializeField(DescriptorContext context, Descriptor descriptor, FieldMetadata f) {
        int value;
        try {
            value = ((Number) f.javaField.get(descriptor)).intValue();
        } catch (Exception e) {
            throw new RuntimeException("Failed to read field " + f.javaField.getName(), e);
        }

        byte[] serialized = serializeType(f.type, value);

        String pretty;
        if (f.printer != null) {
            pretty = f.printer.printField(context.speed(), descriptor, value);
        } else {
            pretty = switch (f.type) {
                case UINT8, UINT16 -> String.valueOf(value);
                case UINT8_HEX -> String.format("0x%02X", value);

                case UINT8_STR -> throw new RuntimeException(); // UINT8_STR must not reach here

                case UINT16_HEX -> String.format("0x%04X", value);
                case UINT16_BCD -> String.format("0x%04X\t%d.%d%d", value, value >> 8, (value >> 4) & 0xF, value & 0xF);
            };
        }

        return new SerializedField(f.name, pretty, serialized);
    }

    private static byte[] serializeType(FieldType type, int value) {
        return switch (type.length) {
            case 1 -> serializeU8(value);
            case 2 -> serializeU16(value);
            default -> throw new RuntimeException("Unknown type length: " + type.length + " for " + type);
        };
    }

    private static byte[] serializeU16(int value) {
        if (value < 0 || value > 65535) {
            throw new IllegalArgumentException("Value is out of range for uint16_t: " + value);
        }

        return new byte[] {
                (byte) (value & 0xFF),
                (byte) (value >> 8)
        };
    }

    private static byte[] serializeU8(int value) {
        if (value < 0 || value > 255) {
            throw new IllegalArgumentException("Value is out of range for uint8_t: " + value);
        }

        return new byte[] { (byte) value };
    }


    private static ClassMetadata computeMetadata(Class<? extends Descriptor> clazz) {
        if (!clazz.isAnnotationPresent(DescriptorClass.class)) {
            throw new IllegalArgumentException("@DescriptorClass annotation is missing on class " + clazz.getName());
        }

        LinkedList<Class<?>> parentClasses = new LinkedList<>();
        Set<Class<?>> seenClasses = new HashSet<>();
        boolean changedAnything = true;

        parentClasses.add(clazz);
        while (changedAnything) {
            Class<?> cls = parentClasses.getFirst();
            changedAnything = false;

            Class<?> superCls = cls.getSuperclass();
            if (superCls != null && !seenClasses.contains(superCls)) {
                parentClasses.addFirst(superCls);
                seenClasses.add(superCls);
                changedAnything = true;
            }

            for (Class<?> itf: cls.getInterfaces()) {
                if (seenClasses.contains(itf)) {
                    continue;
                }

                parentClasses.addFirst(itf);
                seenClasses.add(itf);
                changedAnything = true;
            }
        }

        List<TypeField> typeFields = collectTypeFields(parentClasses);
        List<FieldMetadata> fields = collectFields(parentClasses);

        return new ClassMetadata(clazz.getSimpleName(), typeFields, fields);
    }

    private static List<TypeField> collectTypeFields(LinkedList<Class<?>> parentClasses) {
        List<TypeField> ret = new ArrayList<>();

        Class<?> nextFieldSource = null;
        DescriptorClass.NextTypeField nextField = null;

        for (Class<?> cls: parentClasses) {
            if (!cls.isAnnotationPresent(DescriptorClass.class)) {
                continue;
            }

            DescriptorClass dc = cls.getAnnotation(DescriptorClass.class);

            if (ret.isEmpty()) {
                ret.add(new TypeField(DESC_TYPE_FIELD, FieldType.UINT8_HEX, dc.type(), dc.typeName()));
            } else {
                if (nextField == null) {
                    throw new RuntimeException("Duplicate @DescriptorClass annotation without intermediate " +
                            "@DescriptorClass.NextTypeField: in class " + cls.getName());
                }

                ret.add(new TypeField(nextField.name(), nextField.type(), dc.type(), dc.typeName()));
            }

            if (!dc.nextField().name().isEmpty()) {
                nextFieldSource = cls;
                nextField = dc.nextField();
            } else {
                nextFieldSource = null;
                nextField = null;
            }
        }

        if (nextField != null) {
            throw new RuntimeException("Incomplete @DescriptorClass sequence: @DescriptorClass.NextTypeField in " +
                    "class " + nextFieldSource.getName() + " was not completed in " +
                    parentClasses.getLast().getName());
        }

        if (ret.isEmpty()) {
            throw new RuntimeException("No @DescriptorClass annotation has been found on class " +
                    parentClasses.getLast().getName());
        }

        return List.copyOf(ret);
    }

    private static List<FieldMetadata> collectFields(LinkedList<Class<?>> parentClasses) {
        LinkedList<FieldMetadata> fields = new LinkedList<>();

        for (Class<?> cls: parentClasses) {
            for (Field jf: cls.getFields()) {
                if (!jf.isAnnotationPresent(DescriptorField.class)) {
                    continue;
                }

                DescriptorField descField = jf.getAnnotation(DescriptorField.class);
                String name = descField.name().isEmpty() ? jf.getName() : descField.name();

                FieldPrinter printer = null;
                if (descField.printer() != FieldPrinter.class) {
                    try {
                        printer = descField.printer().getConstructor().newInstance();
                    } catch (Exception e) {
                        throw new RuntimeException(String.format(
                                "Failed to instantiate field printer %s for field %s",
                                descField.printer(), jf
                        ));
                    }
                }

                fields.add(new FieldMetadata(name, jf, descField.value(), printer));
            }
        }

        return List.copyOf(fields);
    }

    private record TypeField(String name, FieldType type, int value, String valueText) {}
    private record FieldMetadata(String name, Field javaField, FieldType type, FieldPrinter printer) {}
    private record ClassMetadata(String className, List<TypeField> typeFields, List<FieldMetadata> fields) {}

    private static final Map<Class<? extends Descriptor>, ClassMetadata> metadataCache = new HashMap<>();
}
