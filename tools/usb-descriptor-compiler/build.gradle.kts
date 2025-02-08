plugins {
    id("java")
    id("maven-publish")
    id("io.freefair.lombok") version "8.4"
}

group = "mx.m-k.microbits"

repositories {
    mavenCentral()
}

sourceSets {
    val api = create("api")
    val plugins = create("plugins")
    val main = main.get()

    // Source set with actual generation logic
    main.apply {
        compileClasspath += api.runtimeClasspath
        runtimeClasspath += api.runtimeClasspath
    }

    // Separate source set to ensure that even built-in plugins only see API classes, and not internals
    plugins.apply {
        compileClasspath += api.runtimeClasspath
        runtimeClasspath += api.runtimeClasspath
    }

    // Joined source set (with no actual sources) to run the code in IDE
    create("all") {
        runtimeClasspath += api.runtimeClasspath
        runtimeClasspath += main.runtimeClasspath
        runtimeClasspath += plugins.runtimeClasspath
    }
}

dependencies {
    val apiDep = configurations.getByName("apiImplementation")

    apiDep("com.google.code.findbugs:jsr305:3.0.2")

    implementation("net.sf.jopt-simple:jopt-simple:5.0.4")
    implementation("org.snakeyaml:snakeyaml-engine:2.7")
}

tasks.test {
    useJUnitPlatform()
}

tasks.compileJava {
    sourceCompatibility = "17"
    targetCompatibility = "17"
}

tasks.jar {
    from(sourceSets.getByName("api").output)
    from(sourceSets.getByName("plugins").output)
}

val apiJar: Jar = tasks.create("apiJar", Jar::class.java) {
    archiveAppendix.set("api")

    from(sourceSets.getByName("api").output)
}

val apiSourcesJar: Jar = tasks.create("apiSourcesJar", Jar::class.java) {
    archiveAppendix.set("api")
    archiveClassifier.set("sources")
    from(sourceSets.getByName("api").allSource)
}

val fatJar: Jar = tasks.create("fatJar", Jar::class.java) {
    archiveClassifier.set("fat")
    dependsOn(tasks.jar)

    manifest {
        attributes(
            "Main-Class" to "microbits.usbd.core.DescriptorCompiler"
        )
    }

    from(configurations.runtimeClasspath.get().map { if (!it.isDirectory) zipTree(it) else it })
    with(tasks.jar.get())
}

val cmakeTargetFileName = project.properties["cmakeOutputJar"]
if (cmakeTargetFileName != null) {
    tasks.create("cmakeJar", Copy::class.java) {
        dependsOn(fatJar)

        val targetFile = project.file(cmakeTargetFileName)
        from(fatJar.archiveFile)
        into(targetFile.parentFile)
        rename { targetFile.name }
    }
}

tasks.build {
    dependsOn(apiJar, fatJar)
}

publishing {
    publications {
        create<MavenPublication>("maven") {
            groupId = "mx.m-k.microbits"
            artifactId = "usb-descriptor-compiler-api"

            artifact(apiJar)
            artifact(apiSourcesJar)
        }
    }

    repositories {
        maven {
            name = "mkMainRepo"
            url = uri("https://maven.m-k.mx")
            credentials(PasswordCredentials::class)
        }
    }
}
