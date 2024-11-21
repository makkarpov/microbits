package microbits.usbd.core.codegen;

import microbits.usbd.core.descriptor.SerializedDescriptor;
import microbits.usbd.core.descriptor.SerializedField;
import microbits.usbd.core.util.PrinterUtils;

import java.io.IOException;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashSet;
import java.util.Set;

public class CodePrintStream implements AutoCloseable {
    public static final int MAX_WIDTH = 80;

    private final PrintStream s;

    private final Set<String> includedFiles = new HashSet<>();

    public CodePrintStream(Path file) throws IOException  {
        this.s = new PrintStream(Files.newOutputStream(file), false, StandardCharsets.UTF_8);
    }

    public void println(Object x) {
        s.println(x);
    }

    public void printComment(String content, boolean banner) {
        if (banner) {
            s.println("/* " + "*".repeat(MAX_WIDTH - 3));
        } else {
            s.println("/*");
        }

        s.print(content);

        if (!content.endsWith("\n")) {
            s.println();
        }

        if (banner) {
            s.println("*".repeat(MAX_WIDTH - 3) + " */");
        } else {
            s.println("*/");
        }
    }

    public void printEmptyLine() {
        s.println();
    }

    public void printInclude(String file) {
        if (includedFiles.contains(file)) {
            return;
        }

        s.println("#include <" + file + ">");
        includedFiles.add(file);
    }

    public void markAsIncluded(String file) {
        includedFiles.add(file);
    }

    public void writeDescriptorData(SerializedDescriptor d, String prefix, boolean last) {
        boolean firstComment = true;
        for (SerializedField f: d.fields()) {
            StringBuilder line = new StringBuilder();
            line.append(prefix).append(firstComment ? "/* " : " * ");
            appendFieldText(line, f.fieldName() + "\t" + f.prettyValue());
            s.println(line);

            firstComment = false;
        }

        if (d.prettyTrailer() != null && !d.prettyTrailer().isEmpty()) {
            for (String trailer: d.prettyTrailer().split("\n")) {
                if (trailer.isEmpty()) {
                    continue;
                }

                StringBuilder line = new StringBuilder();
                line.append(prefix).append(firstComment ? "/* " : " * ");
                appendFieldText(line, trailer);
                s.println(line);

                firstComment = false;
            }
        }

        s.println(prefix + " *" + "*".repeat(MAX_WIDTH - prefix.length() - 3) + "/");

        int bytesPerLine = (MAX_WIDTH - prefix.length()) / 6; // '0x11, '
        byte[] data = d.toByteArray();
        int offset = 0;

        while (offset < data.length) {
            int chunk = Math.min(bytesPerLine, data.length - offset);

            StringBuilder line = new StringBuilder();
            line.append(prefix);

            for (int i = 0; i < chunk; i++) {
                PrinterUtils.appendHexByte(line, data[offset + i]);

                if (!last || (offset + i != data.length - 1)) {
                    line.append(',');
                }

                if (i != chunk - 1) {
                    line.append(' ');
                }
            }

            s.println(line);
            offset += chunk;
        }

        if (!last) {
            s.println();
        }
    }

    private void appendFieldText(StringBuilder ret, String line) {
        String[] parts = line.split("\t", 3);

        if (parts.length > 1) {
            appendFixedLen(ret, parts[0], FIELD_NAME_LEN);

            if (parts.length > 2) {
                appendFixedLen(ret, parts[1], FIELD_VALUE_LEN);
                ret.append(parts[2]);
            } else {
                ret.append(parts[1]);
            }
        } else {
            ret.append(parts[0]);
        }
    }

    private void appendFixedLen(StringBuilder ret, String value, int expectedLength) {
        ret.append(value);

        if (value.length() >= expectedLength) {
            ret.append(' ');
            return;
        }

        ret.append(" ".repeat(expectedLength - value.length()));
    }

    @Override
    public void close() {
        s.close();
    }

    private static final int FIELD_NAME_LEN = 24;
    private static final int FIELD_VALUE_LEN = 10;
}
