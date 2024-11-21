package microbits.usbd.core.codegen;

import microbits.usbd.api.codegen.ConfigProperty;
import microbits.usbd.api.codegen.TargetCodeGenerator;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.core.DescriptorCompiler;
import microbits.usbd.core.state.DescriptorState;
import microbits.usbd.core.state.FunctionState;

import javax.annotation.Nullable;
import java.util.*;
import java.util.function.ToIntFunction;
import java.util.stream.Collectors;

public class ConfigChecksGenerator {
    private final DescriptorCompiler compiler;
    private final List<ConfigCheckValue> checkValues;

    private final TargetCodeGenerator targetCodeGenerator;

    public ConfigChecksGenerator(DescriptorCompiler compiler, TargetCodeGenerator targetCodeGenerator) {
        this.compiler = compiler;
        this.targetCodeGenerator = targetCodeGenerator;

        this.checkValues = generateConfigChecks();
    }

    private List<ConfigCheck> collectConfigChecks() {
        List<ConfigCheck> allChecks = new ArrayList<>();

        int maxInEndpoints = maxDescriptorValue(d -> d.endpointAllocation.numInEndpoints());
        allChecks.add(new ConfigCheck(null, ConfigProperties.IN_ENDPOINTS, maxInEndpoints));

        int maxOutEndpoints = maxDescriptorValue(d -> d.endpointAllocation.numOutEndpoints());
        allChecks.add(new ConfigCheck(null, ConfigProperties.OUT_ENDPOINTS, maxOutEndpoints));

        allChecks.add(new ConfigCheck(null, ConfigProperties.FUNCTIONS, compiler.functions().size()));

        int maxInterfaces = maxDescriptorValue(d -> d.interfaces.numInterfaces());
        allChecks.add(new ConfigCheck(null, ConfigProperties.INTERFACES, maxInterfaces));

        int funcEndpoints = maxDescriptorValue(d -> d.endpointAllocation.maxFuncEndpoints());
        allChecks.add(new ConfigCheck(null, ConfigProperties.FUNC_ENDPOINTS, funcEndpoints));

        allChecks.add(new ConfigCheck(null, ConfigProperties.CONTROL_PACKET, compiler.spec.controlPipe.maxPacket));

        if (compiler.spec.device.runtimeSerial) {
            allChecks.add(new ConfigCheck(null, ConfigProperties.RUNTIME_SERIAL_NR, 1));
        }

        if (compiler.speed() == LinkSpeed.HIGH) {
            allChecks.add(new ConfigCheck(null, ConfigProperties.HIGH_SPEED, 1));
        }

        for (FunctionState st: compiler.functions()) {
            if (st.codeGenerator == null) {
                continue;
            }

            for (ConfigProperty.Check c: st.codeGenerator.configurationChecks()) {
                allChecks.add(new ConfigCheck(st, c.property, c.value));
            }
        }

        for (ConfigProperty.Check c: targetCodeGenerator.configurationChecks()) {
            allChecks.add(new ConfigCheck(null, c.property, c.value));
        }

        return allChecks;
    }

    private List<ConfigCheckValue> generateConfigChecks() {
        List<ConfigCheck> allChecks = collectConfigChecks();
        List<ConfigCheckKey> keys = allChecks.stream()
                .collect(Collectors.groupingBy(c -> c.property.name))
                .entrySet().stream()
                .map(e -> new ConfigCheckKey(e.getKey(), e.getValue()))
                .sorted(Comparator.comparingInt(ConfigCheckKey::sortOrder)
                        .thenComparing(ConfigCheckKey::key))
                .toList();

        List<ConfigCheckValue> ret = new ArrayList<>();

        for (ConfigCheckKey k: keys) {
            Set<ConfigProperty.Type> types = k.checks.stream()
                    .map(c -> c.property.type)
                    .collect(Collectors.toUnmodifiableSet());

            if (types.size() != 1) {
                throw new IllegalArgumentException(String.format("Config property '%s' has conflicting types: %s",
                        k.key, types));
            }

            ConfigProperty.Type type = types.iterator().next();
            CheckTypeHandler typeHandler = TYPE_HANDLERS.get(type);

            int reducedValue;
            try {
                reducedValue = typeHandler.reduceValue(k.checks);
            } catch (Exception e) {
                throw new IllegalArgumentException(String.format("Failed to combine values for property '%s': %s",
                        k.key, e.getMessage()), e);
            }

            ConfigProperty prop = k.checks.get(0).property;
            ret.add(new ConfigCheckValue(prop, reducedValue));
        }

        return ret;
    }

    public void writeSuggestedValues(StringBuilder ret) {
        for (ConfigCheckValue v: checkValues) {
            ret.append("#define ");
            ret.append(v.property.name);
            ret.append(" ".repeat(56 - v.property.name.length()));
            ret.append(v.value);
            ret.append("\n");
        }
    }

    public void writeChecks(CodePrintStream output) {
        for (ConfigCheckValue v: checkValues) {
            CheckTypeHandler handler = TYPE_HANDLERS.get(v.property.type);
            String key = v.property.name;

            output.println("#if " + handler.getExpression(key, v.value));
            output.println("#error \"" + handler.getErrorMessage(key, v.value) + "\"");
            output.println("#endif // " + key);
            output.printEmptyLine();
        }
    }

    private int maxDescriptorValue(ToIntFunction<DescriptorState> fn) {
        return compiler.descriptors().values().stream()
                .mapToInt(fn).max().orElse(0);
    }

    private record ConfigCheck(@Nullable FunctionState origin, ConfigProperty property, int value) {}

    private record ConfigCheckKey(String key, List<ConfigCheck> checks) {
        public int sortOrder() {
            return checks.stream()
                    .mapToInt(c -> c.origin == null ? 0 : c.origin.id + 1)
                    .min().orElse(0);
        }
    }

    private record ConfigCheckValue(ConfigProperty property, int value) {}

    private static final Map<ConfigProperty.Type, CheckTypeHandler> TYPE_HANDLERS = Map.of(
            ConfigProperty.Type.FLAG, new FlagTypeHandler(),
            ConfigProperty.Type.VALUE, new ValueTypeHandler(),
            ConfigProperty.Type.SIZE, new SizeTypeHandler()
    );

    private interface CheckTypeHandler {
        int reduceValue(List<ConfigCheck> checks);

        String getExpression(String key, int value);

        String getErrorMessage(String key, int value);
    }

    private static abstract class ExactValueTypeHandler implements CheckTypeHandler {
        @Override
        public int reduceValue(List<ConfigCheck> checks) {
            Set<Integer> values = checks.stream().map(c -> c.value).collect(Collectors.toUnmodifiableSet());
            if (values.size() != 1) {
                throw new IllegalArgumentException("Property has conflicting values: " + values);
            }

            return values.iterator().next();
        }
    }

    private static class FlagTypeHandler extends ExactValueTypeHandler {
        @Override
        public String getExpression(String key, int value) {
            // N.B. Inverted logic since we are testing for errors
            return value == 0 ? key : "!" + key;
        }

        @Override
        public String getErrorMessage(String key, int value) {
            if (value != 0) {
                return String.format("Configuration flag '%s' must be set", key);
            } else {
                return String.format("Configuration flag '%s' must not be set", key);
            }
        }
    }

    private static class ValueTypeHandler extends ExactValueTypeHandler {
        @Override
        public String getExpression(String key, int value) {
            return String.format("%s != %d", key, value);
        }

        @Override
        public String getErrorMessage(String key, int value) {
            return String.format("Configuration value '%s' must be %d", key, value);
        }
    }

    private static class SizeTypeHandler implements CheckTypeHandler {
        @Override
        public int reduceValue(List<ConfigCheck> checks) {
            return checks.stream().mapToInt(c -> c.value).max().orElse(0);
        }

        @Override
        public String getExpression(String key, int value) {
            return String.format("%s < %d", key, value);
        }

        @Override
        public String getErrorMessage(String key, int value) {
            return String.format("Configuration value '%s' must be at least %d", key, value);
        }
    }
}
