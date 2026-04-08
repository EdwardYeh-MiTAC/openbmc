project('powercap_monitor', 'cpp',
  default_options : ['cpp_std=c++23']
)

sdbusplus_dep = dependency('sdbusplus')
systemd_dep = dependency('libsystemd')
phosphor_dbus_dep = dependency('phosphor-dbus-interfaces')
phosphor_logging_dep = dependency('phosphor-logging')

executable(
  'powercap_monitor',
  ['monitor-main.cpp', 'powercap-monitor.cpp'],
  dependencies: [
    sdbusplus_dep,
    phosphor_logging_dep,
    phosphor_dbus_dep,
    systemd_dep,
  ],
  install: true,
  install_dir: '/usr/bin'
)