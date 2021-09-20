project('mccc', 'cpp')

add_project_arguments([], language: 'cpp')
add_project_link_arguments([], language: 'cpp')

dep = [
	dependency('boost', modules : ['spirit'])
]

inc = [
	'.'
]

src = [
	'main.cpp',
	'Asm.cpp',
	'AsmAST.cpp',
	'Sym.cpp',
	'SymAST.cpp',
	'Processor.cpp',
	'Simulator.cpp',
	'AutomatedTests.cpp'
]

exe = executable(
	'mccc',
	sources : src,
	dependencies : dep,
	include_directories : inc,
	install : true,
	override_options : ['cpp_std=c++11']
)

test('MCCC Automated Test', exe, args : ['automatedTest'])
