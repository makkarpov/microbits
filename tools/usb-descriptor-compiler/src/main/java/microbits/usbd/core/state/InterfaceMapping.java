package microbits.usbd.core.state;

import lombok.AllArgsConstructor;
import microbits.usbd.api.iface.InterfaceDefinition;

import java.util.List;

@SuppressWarnings("ClassCanBeRecord")
@AllArgsConstructor
public class InterfaceMapping {
    public final List<FnIntf> physical;

    public final List<FnData> functions;

    public int numInterfaces() {
        return physical.size();
    }

    public record FnIntf(FunctionState st, InterfaceDefinition defn) {}
    public record FnData(int interfaceBase, List<InterfaceDefinition> defs) {}
}
