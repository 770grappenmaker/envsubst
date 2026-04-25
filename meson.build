project('envsubst', 'c')

bash = find_program('bash')

progs = {'envsubst': 'envsubst.c', 'envsubst-go': 'envsubst_go.c'}

foreach prog, source : progs
    e = executable(prog, source, install: true)

    conf_data = configuration_data()
    conf_data.set('ENVSUBST', e.full_path())
    conf_data.set('TESTS_DIR', meson.project_source_root() / 'tests')
    tester = configure_file(
        input: 'test.sh.in',
        output: 'test-' + prog + '.sh',
        configuration: conf_data,
    )

    test(prog, bash, args: [tester], depends: [e], protocol: 'tap')
endforeach
