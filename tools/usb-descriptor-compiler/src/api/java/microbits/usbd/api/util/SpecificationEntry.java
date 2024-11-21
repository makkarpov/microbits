package microbits.usbd.api.util;

import javax.annotation.Nullable;
import java.util.List;

public interface SpecificationEntry {
    /** @return Whether key is contained in the mapping */
    boolean has(String key);

    /** @return Boolean with given key. Throws if value is missing. */
    boolean bool(String key);

    /** @return Boolean with given key, or {@code null} if value is missing. */
    @Nullable
    Boolean boolOpt(String key);

    /** @return String with given key. Throws if value is missing. */
    String string(String key);

    /** @return String with given key, or {@code null} if value is missing. */
    @Nullable
    String stringOpt(String key);

    /** @return Enumeration value (case-insensitive) with given key. Throws if value is missing. */
    <E extends Enum<E>> E enumeration(String key, Class<E> enumClass);

    /** @return Enumeration value (case-insensitive) with given ke, or {@code null} if value is missing. */
    @Nullable
    <E extends Enum<E>> E enumerationOpt(String key, Class<E> enumClass);

    /** @return List of strings with given key. Throws if value is missing. */
    List<String> stringList(String key);

    /** @return List of strings with given key, or {@code null} if value is missing. */
    @Nullable
    List<String> stringListOpt(String key);

    /** @return Integer with given key. Throws if value is missing */
    int integer(String key);

    /** @return Integer with given key, or {@code null} if value is missing. */
    @Nullable
    Integer integerOpt(String key);

    /** @return Object with given key. Throws if value is missing. */
    SpecificationEntry object(String key);

    /** @return Object with given key, or empty object if value is missing. */
    @Nullable
    SpecificationEntry objectOpt(String key);

    /** @return List of objects with given key. Throws if value is missing. */
    List<? extends SpecificationEntry> objectList(String key);

    /** @return List of objects with given key, or {@code null} if value is missing. */
    @Nullable
    List<? extends SpecificationEntry> objectListOpt(String key);

    /** @return Raw value associated with the key, or {@code null} if value is missing. */
    @Nullable
    Object rawValue(String key);

    /** Create specification parsing error related to specified key. */
    RuntimeException createError(String key, String message);
}
