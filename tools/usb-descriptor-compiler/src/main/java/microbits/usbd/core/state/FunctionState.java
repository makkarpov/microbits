package microbits.usbd.core.state;

import microbits.usbd.api.codegen.FunctionCodeGenerator;
import microbits.usbd.api.plugin.FunctionInstance;
import microbits.usbd.api.plugin.USBFunction;
import microbits.usbd.api.plugin.FunctionEnvironment;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.core.DescriptorCompiler;

import javax.annotation.Nullable;

public class FunctionState implements FunctionInstance {
    public final int id;

    /** User-defined function name */
    public final String name;

    public final DescriptorCompiler compiler;

    /** Function implementation from the plugin */
    public USBFunction impl;

    public final FunctionEnvironment env;

    @Nullable
    public FunctionCodeGenerator codeGenerator;

    public FunctionState(int id, String name, DescriptorCompiler compiler) {
        this.id = id;
        this.name = name;
        this.compiler = compiler;

        this.env = new EnvironmentImpl();
    }

    @Override
    public int identifier() {
        return id;
    }

    @Override
    public String name() {
        return name;
    }

    @Override
    public String toString() {
        return "FunctionState{id=%d, name='%s', impl=[0x%08X] %s}".formatted(id, name, impl.hashCode(), impl);
    }

    private class EnvironmentImpl implements FunctionEnvironment {
        @Override
        public LinkSpeed speed() {
            return compiler.speed();
        }

        @Override
        public String toString() {
            return "FunctionEnvironment(id=%d, func='%s')".formatted(id, name);
        }
    }
}
