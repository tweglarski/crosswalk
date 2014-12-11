{
  'targets': [
    {
      'target_name': 'widget-manifest-parser',
      'type': 'shared_library',
      'dependencies': [
        '../base/base.gyp:base',
        'build/system.gyp:tizen',
        'application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'sources': [
        'widget-manifest-parser.cc',
        'widget-manifest-parser.h',
        '../../runtime/common/xwalk_system_locale.cc',
        '../../runtime/common/xwalk_system_locale.h',
      ],
      'include_dirs': [
        '..',
        '../..',
        '../../..',
      ],
    },
  ],
}
