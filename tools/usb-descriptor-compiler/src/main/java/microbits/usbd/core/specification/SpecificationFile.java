package microbits.usbd.core.specification;

import microbits.usbd.api.target.LinkSpeed;
import org.snakeyaml.engine.v2.api.Load;
import org.snakeyaml.engine.v2.api.LoadSettings;

import javax.annotation.Nullable;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class SpecificationFile {
    /** Name of generated descriptor variable, optionally namespaced with '::' delimiters */
    public final String name;

    /** Extra include files for the generated code */
    public final List<String> includes;

    /** Target hardware name */
    public final String target;

    /** Specified link speed */
    @Nullable
    public final LinkSpeed speed;

    /** Control pipe specification */
    public final ControlPipeSpecification controlPipe;

    /** Device specification */
    public final DeviceSpecification device;

    /** Function specifications */
    public final List<FunctionSpecification> functions;

    private SpecificationFile(Map<?, ?> yamlData) {
        ParsedObject parser = new ParsedObject("", yamlData);

        name = parser.string("name");
        includes = Objects.requireNonNullElse(parser.stringListOpt("includes"), List.of());
        target = parser.string("target");
        speed = parser.enumerationOpt("speed", LinkSpeed.class);

        controlPipe = new ControlPipeSpecification(parser.objectOpt("controlPipe"));
        device = new DeviceSpecification(parser.object("device"));

        functions = parser.objectList("functions").stream()
                .map(FunctionSpecification::new)
                .toList();
    }

    public static SpecificationFile load(Path path) throws IOException {
        LoadSettings settings = LoadSettings.builder().build();
        Object result;

        try (InputStream is = Files.newInputStream(path)) {
            result = new Load(settings).loadFromInputStream(is);
        }

        if (!(result instanceof Map<?, ?>)) {
            throw new RuntimeException("Specification file must be an YAML object");
        }

        return new SpecificationFile((Map<?, ?>) result);
    }

    @Override
    public String toString() {
        return "SpecificationFile{" +
                "name='" + name + '\'' +
                ", includes=" + includes +
                ", device=" + device +
                ", functions=" + functions +
                '}';
    }
}
