package microbits.usbd.api.codegen;

import javax.annotation.Nullable;
import java.util.List;

public interface TargetCodeGenerator {
    /** Target identifier to match against runtime implementation. */
    String targetIdentifier();

    /** @return List of file names that should be included in the generated output */
    default List<String> includeFiles() {
        return List.of();
    }

    /** @return List of configuration checks */
    default List<ConfigProperty.Check> configurationChecks() {
        return List.of();
    }

    /**
     * Generate a target configuration structure, using a provided symbol name.
     *
     * <p>Generated symbol should have `const static` modifiers, and should not be an array. Generated symbol will be
     * referenced as {@code &amp;symbolName} at the later stages.</p>
     *
     * @param symbolName Symbol name to use
     * @return COde with generated symbol
     */
    @Nullable
    default String generateConfiguration(String symbolName, TargetConfigurationData data) {
        return null;
    }
}
