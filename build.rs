fn main() {
    cbindgen::Builder::new()
        .with_src("./src/ffi.rs")
        .with_language(cbindgen::Language::C)
        .with_tab_width(4)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("bindings.h");
}
