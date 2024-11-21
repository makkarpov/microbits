package microbits.usbd.core.util;

import joptsimple.OptionParser;
import joptsimple.OptionSpec;
import joptsimple.OptionException;
import joptsimple.OptionSet;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Set;
import java.util.stream.Collectors;

public class CompilerOptions {
    /** Set of plugins specified in command line */
    public final Set<Path> pluginFiles;

    /** Specification input file */
    public final Path specificationFile;

    /** Compiled output file */
    public final Path outputFile;

    public CompilerOptions(String[] args) {
        OptionParser parser = new OptionParser();

        OptionSpec<String> plugins = parser.accepts("plugins").withRequiredArg().withValuesSeparatedBy(':');
        OptionSpec<String> specification = parser.accepts("specification").withRequiredArg().required();
        OptionSpec<String> out = parser.accepts("out").withRequiredArg().required();

        OptionSet opts;
        try {
            opts = parser.parse(args);
        } catch (OptionException e) {
            System.err.println("Failed to parse options:");
            System.err.println(e.getMessage());
            System.exit(1);

            throw e; // to make sure that compiler recognizes this as dead branch
        }

        pluginFiles = opts.valuesOf(plugins).stream()
                .filter(s -> !s.isEmpty())
                .map(Paths::get)
                .collect(Collectors.toUnmodifiableSet());

        specificationFile = Paths.get(opts.valueOf(specification));
        outputFile = Paths.get(opts.valueOf(out));

        try {
            validate();
        } catch (Exception e) {
            System.err.println("Failed to validate options:");
            System.err.println(e.getMessage());
            System.exit(1);

            throw e;
        }
    }

    private void validate() {
        Set<Path> missingPlugins = pluginFiles.stream()
                .map(Path::toAbsolutePath)
                .filter(f -> !Files.exists(f) || !Files.isRegularFile(f))
                .collect(Collectors.toUnmodifiableSet());

        if (!missingPlugins.isEmpty()) {
            String joinedPlugins = missingPlugins.stream().map(Path::toString).collect(Collectors.joining(", "));
            throw new IllegalArgumentException("Plugin resolution failed: " + joinedPlugins);
        }

        if (!Files.exists(specificationFile) || !Files.isRegularFile(specificationFile)) {
            throw new IllegalArgumentException("Specification file not found: " + specificationFile.toAbsolutePath());
        }
    }
}
