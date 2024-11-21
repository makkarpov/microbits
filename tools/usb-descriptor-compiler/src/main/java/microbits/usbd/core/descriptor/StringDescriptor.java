package microbits.usbd.core.descriptor;

import lombok.AllArgsConstructor;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.core.util.PrinterUtils;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.stream.Collectors;


public interface StringDescriptor extends Descriptor {
    int TYPE_STRING = 0x03;

    /** String descriptor representation */
    String repr();

    @AllArgsConstructor
    @SuppressWarnings("ClassCanBeRecord")
    @DescriptorClass(type = TYPE_STRING, typeName = "String (language list)")
    final class LanguageList implements StringDescriptor {
        public final List<Integer> languages;

        @Override
        public byte[] serialize() {
            byte[] r = new byte[languages.size() * 2];

            for (int i = 0, j = 0; i < languages.size(); i++) {
                int l = languages.get(i);
                r[j++] = (byte) l;
                r[j++] = (byte) (l >> 8);
            }

            return r;
        }

        @Override
        public String repr() {
            return languages.stream()
                    .map(l -> String.format("0x%04X", l))
                    .collect(Collectors.joining(", "));
        }

        @Override
        public String prettyPrint() {
            return "wLangId\t" + repr();
        }

        @Override
        public String toString() {
            return String.format("LanguageList{%s}", repr());
        }
    }

    @AllArgsConstructor
    @SuppressWarnings("ClassCanBeRecord")
    @DescriptorClass(type = TYPE_STRING, typeName = "String")
    final class StringData implements StringDescriptor {
        public final String s;

        @Override
        public byte[] serialize() {
            return s.getBytes(StandardCharsets.UTF_16LE);
        }

        @Override
        public String repr() {
            return PrinterUtils.stringRepr(s);
        }

        @Override
        public String prettyPrint() {
            return "wString\t" + repr();
        }

        @Override
        public String toString() {
            return String.format("StringData{%s}", repr());
        }
    }

    /**
     * Placeholder for runtime-generated serial number, which is not included in static descriptor set but has its
     * own index.
     */
    final class RuntimePlaceholder implements StringDescriptor {
        @Override
        public byte[] serialize() {
            throw new UnsupportedOperationException("RuntimePlaceholder cannot be serialized");
        }

        @Override
        public String repr() {
            return "<runtime descriptor>";
        }

        @Override
        public String toString() {
            return repr();
        }
    }
}
