package microbits.usbd.core.specification;

import microbits.usbd.api.util.SpecificationEntry;

import javax.annotation.Nullable;
import java.util.*;
import java.util.stream.Collectors;

public class ParsedObject implements SpecificationEntry {
    private static final Set<String> TRUE_STRINGS = Set.of("true", "yes", "y", "on");
    private static final Set<String> FALSE_STRINGS = Set.of("false", "no", "n", "off");

    private final String path;
    private final Map<?, ?> data;

    public ParsedObject(String path, Object data) {
        if (!(data instanceof Map<?, ?>)) {
            throw createInvalidType(path, "an object", data);
        }

        this.path = path;
        this.data = Collections.unmodifiableMap((Map<?, ?>) data);
    }

    @Override
    public boolean has(String key) {
        return data.containsKey(key);
    }

    @Override
    public boolean bool(String key) {
        ensureValuePresent(key);

        Object value = data.get(key);
        if (value instanceof Boolean) {
            return (Boolean) value;
        }

        if (value instanceof Number) {
            int i = ((Number) value).intValue();
            if (i < 0 || i > 1) {
                throw createError(key, "invalid boolean value: numbers must be either 0 or 1");
            }

            return i != 0;
        }

        if (value instanceof String s) {
            if (TRUE_STRINGS.contains(s.toLowerCase())) {
                return true;
            }

            if (FALSE_STRINGS.contains(s.toLowerCase())) {
                return false;
            }

            throw createError(key, "invalid boolean value: " + s);
        }

        throw createInvalidType(key, "a boolean", value);
    }

    @Override
    @Nullable
    public Boolean boolOpt(String key) {
        return data.containsKey(key) ? bool(key) : null;
    }

    @Override
    public String string(String key) {
        ensureValuePresent(key);
        return data.get(key).toString();
    }

    @Override
    @Nullable
    public String stringOpt(String key) {
        if (!data.containsKey(key)) {
            return null;
        }

        return data.get(key).toString();
    }

    @Override
    public <E extends Enum<E>> E enumeration(String key, Class<E> enumClass) {
        String strValue = string(key);

        for (E value: enumClass.getEnumConstants()) {
            if (strValue.equalsIgnoreCase(value.name())) {
                return value;
            }
        }

        String validValues = Arrays.stream(enumClass.getEnumConstants())
                .map(Enum::name).collect(Collectors.joining(", "));

        throw createError(key, String.format("undefined enumeration constant: %s. Valid values are: %s",
                strValue, validValues));
    }

    @Override
    @Nullable
    public <E extends Enum<E>> E enumerationOpt(String key, Class<E> enumClass) {
        return data.containsKey(key) ? enumeration(key, enumClass) : null;
    }

    @Override
    public List<String> stringList(String key) {
        ensureValuePresent(key);
        return stringListOpt(key);
    }

    @Override
    @Nullable
    public List<String> stringListOpt(String key) {
        if (!data.containsKey(key)) {
            return List.of();
        }

        Object result = data.get(key);
        if (!(result instanceof List<?>)) {
            throw createInvalidType(joinPath(key), "a list of strings", result);
        }

        return ((List<?>) result).stream()
                .map(Object::toString)
                .toList();
    }

    @Override
    public int integer(String key) {
        ensureValuePresent(key);
        return convertInt(key, data.get(key));
    }

    @Override
    @Nullable
    public Integer integerOpt(String key) {
        if (!data.containsKey(key)) {
            return null;
        }

        return convertInt(key, data.get(key));
    }

    @Override
    public ParsedObject object(String key) {
        ensureValuePresent(key);
        return new ParsedObject(joinPath(key), data.get(key));
    }

    @Override
    @Nullable
    public ParsedObject objectOpt(String key) {
        if (!data.containsKey(key)) {
            return new ParsedObject(joinPath(key), Map.of());
        }

        return new ParsedObject(joinPath(key), data.get(key));
    }

    @Override
    public List<ParsedObject> objectList(String key) {
        ensureValuePresent(key);
        String pathBase = joinPath(key);

        Object value = data.get(key);
        if (!(value instanceof List<?> valueList)) {
            throw createInvalidType(pathBase, "a list of objects", value);
        }

        List<ParsedObject> resultList = new ArrayList<>();

        for (int i = 0; i < valueList.size(); i++) {
            resultList.add(new ParsedObject(String.format("%s[%d]", pathBase, i), valueList.get(i)));
        }

        return Collections.unmodifiableList(resultList);
    }

    @Override
    @Nullable
    public List<ParsedObject> objectListOpt(String key) {
        if (!data.containsKey(key)) {
            return null;
        }

        return objectList(key);
    }

    @Override
    @Nullable
    public Object rawValue(String key) {
        return data.get(key);
    }

    private int convertInt(String key, Object value) {
        if (value instanceof Number n) {
            return n.intValue();
        }

        if (value instanceof String s) {
            s = s.toLowerCase();

            try {
                if (s.startsWith("0x")) {
                    return Integer.parseInt(s.substring(2), 16);
                } else if (s.startsWith("0b")) {
                    return Integer.parseInt(s.substring(2), 2);
                } else {
                    return Integer.parseInt(s);
                }
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException(String.format(
                        "Integer value cannot be parsed: key '%s', value '%s'",
                        joinPath(key), s
                ));
            }
        }

        throw createInvalidType(joinPath(key), "an integer", value);
    }

    public String joinPath(String key) {
        if (path.isEmpty()) {
            return key;
        } else {
            return String.format("%s.%s", path, key);
        }
    }

    @Override
    public RuntimeException createError(String key, String message) {
        return new IllegalArgumentException(String.format("'%s': %s", joinPath(key), message));
    }

    private void ensureValuePresent(String key) {
        if (!data.containsKey(key)) {
            throw new IllegalArgumentException(String.format(
                    "Missing required key: '%s'",
                    joinPath(key)
            ));
        }
    }

    @Override
    public String toString() {
        return String.format("ParsedObject(path='%s', data=%s)", path, data);
    }

    private static IllegalArgumentException createInvalidType(String path, String expected, Object actual) {
        return new IllegalArgumentException(String.format(
                "Unexpected type at key '%s'. Expected %s, got %s.",
                path, expected, actual.getClass().getName()
        ));
    }
}
