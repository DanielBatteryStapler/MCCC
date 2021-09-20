(use-modules (guix packages)
	(guix gexp)
	((guix licenses) #:prefix license:)
	(guix build-system meson)
	(gnu packages boost))

(package
	(name "MCCC")
	(version "0.0")
	(inputs
		`(("boost" ,boost)))
	(native-inputs '())
	(propagated-inputs '())
	(source (local-file "./src" #:recursive? #t))
	(build-system meson-build-system)
	(synopsis "MCCC: Minimalist CPU and Compiler Collection")
	(description
		"Fully functional minimalist to high-level programming language stack")
	(home-page "https://github.com/danielbatterystapler/MCCC")
	(license license:gpl2+))

