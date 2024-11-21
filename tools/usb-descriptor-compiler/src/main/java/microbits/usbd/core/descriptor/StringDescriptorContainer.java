package microbits.usbd.core.descriptor;

import microbits.usbd.api.descriptor.StringReference;

import java.util.*;

public class StringDescriptorContainer {
    private final List<StringReferenceImpl> descriptors = new ArrayList<>();
    private final Map<String, StringReferenceImpl> stringCache = new HashMap<>();

    public StringReference putString(Tag tag, String data) {
        StringReferenceImpl r = stringCache.get(data);
        if (r != null) {
            if (r.tag.compareTo(tag) > 0) {
                r.tag = tag;
            }

            return r;
        }

        r = new StringReferenceImpl(tag, new StringDescriptor.StringData(data));
        descriptors.add(r);
        stringCache.put(data, r);

        return r;
    }

    public StringReference putPlaceholder(Tag tag) {
        StringReferenceImpl r = new StringReferenceImpl(tag, new StringDescriptor.RuntimePlaceholder());
        descriptors.add(r);
        return r;
    }

    public void assignIndices() {
        if (descriptors.isEmpty()) {
            return;
        }

        descriptors.add(new StringReferenceImpl(
                Tag.LANG_IDS,
                new StringDescriptor.LanguageList(List.of(0x0409))
        ));

        descriptors.sort(Comparator.comparing(r -> r.tag));

        for (int i = 0; i < descriptors.size(); i++) {
            descriptors.get(i).index = i;
        }
    }

    public int getIndex(StringReference ref) {
        if (!(ref instanceof StringReferenceImpl)) {
            throw new RuntimeException("Unknown string reference class: " + ref.getClass().getName());
        }

        int idx = ((StringReferenceImpl) ref).index;
        if (idx < 0) {
            throw new IllegalStateException("String descriptor indices are not yet assigned");
        }

        return idx;
    }

    public List<IndexedStringDescriptor> descriptors() {
        return descriptors.stream()
                .filter(x -> !(x.descriptor instanceof StringDescriptor.RuntimePlaceholder))
                .sorted(Comparator.comparingInt(x -> x.index))
                .map(x -> new IndexedStringDescriptor(x.index, x.descriptor))
                .toList();
    }

    /** Tags define descriptor ordering in final string descriptor set */
    public enum Tag {
        LANG_IDS,

        MANUFACTURER,
        PRODUCT,
        SERIAL,

        OTHER
    }

    private static class StringReferenceImpl implements StringReference {
        public final StringDescriptor descriptor;

        public Tag tag;
        public int index = -1;

        public StringReferenceImpl(Tag tag, StringDescriptor descriptor) {
            this.tag = tag;
            this.descriptor = descriptor;
        }

        @Override
        public String repr() {
            return descriptor.repr();
        }

        @Override
        public String toString() {
            return String.format("StringReference{index=%d, tag=%s, data=%s}", index, tag, descriptor.repr());
        }
    }
}
