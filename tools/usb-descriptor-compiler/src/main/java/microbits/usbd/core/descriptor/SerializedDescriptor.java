package microbits.usbd.core.descriptor;

import microbits.usbd.api.descriptor.Descriptor;

import javax.annotation.Nullable;
import java.util.List;

public record SerializedDescriptor(
        Descriptor source,
        List<SerializedField> fields,
        @Nullable byte[] trailer,
        @Nullable String prettyTrailer
) {
    public int totalLength() {
        int r = 0;

        for (SerializedField f: fields) {
            r += f.length();
        }

        if (trailer != null) {
            r += trailer.length;
        }

        return r;
    }

    public byte[] toByteArray() {
        byte[] ret = new byte[totalLength()];
        int ofs = 0;

        for (SerializedField f: fields) {
            int len = f.length();
            System.arraycopy(f.serialized(), 0, ret, ofs, len);
            ofs += len;
        }

        if (trailer != null) {
            System.arraycopy(trailer, 0, ret, ofs, trailer.length);
        }

        return ret;
    }
}
