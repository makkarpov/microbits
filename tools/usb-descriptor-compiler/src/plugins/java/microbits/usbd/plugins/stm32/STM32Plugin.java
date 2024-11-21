package microbits.usbd.plugins.stm32;

import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.api.target.TargetFeature;
import microbits.usbd.api.plugin.CompilerPlugin;
import microbits.usbd.api.target.TargetProperties;

import java.util.Map;
import java.util.Set;

public class STM32Plugin implements CompilerPlugin {
    @Override
    public Set<String> targetNames() {
        return TARGETS.keySet();
    }

    @Override
    public TargetProperties lookupTarget(String name) {
        return TARGETS.get(name);
    }

    @Override
    public String toString() {
        return "STM32Plugin";
    }

    public static final TargetProperties STM32G4 = TargetProperties.builder()
            .name("STM32G4xx")
            .inEndpoints(8)
            .outEndpoints(8)
            .targetFeatures(
                    TargetFeature.IN_OUT_SAME_TYPE, TargetFeature.DOUBLE_BUF_UNIDIRECTIONAL,
                    TargetFeature.DOUBLE_BUF_EXPLICIT
            )
            .build();

    public static final TargetProperties STM32U5_FS = TargetProperties.builder()
            .name("STM32U5xx/FS")
            .inEndpoints(6)
            .outEndpoints(6)
            .build();

    public static final TargetProperties STM32U5_HS = TargetProperties.builder()
            .name("STM32U5xx/HS")
            .inEndpoints(9)
            .outEndpoints(9)
            .speeds(LinkSpeed.FULL, LinkSpeed.HIGH)
            .build();

    public static final Map<String, TargetProperties> TARGETS = Map.of(
            "stm32g4",      STM32G4,
            "stm32u5",      STM32U5_FS,
            "stm32u5-fs",   STM32U5_FS,
            "stm32u5-hs",   STM32U5_HS
    );
}
