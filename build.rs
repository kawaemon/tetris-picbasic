fn main() {
    cbindgen::Builder::new()
        .with_src("./src/ffi.rs")
        .with_language(cbindgen::Language::C)
        .with_tab_width(4)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("bindings.h");

    cc::Build::new()
        .out_dir(".")
        .opt_level_str("s")
        .file("src/run.c")
        .compile("liblogic.a");

    println!("cargo:rustc-link-search=.");
    println!("cargo:rustc-link-lib=logic");
}
