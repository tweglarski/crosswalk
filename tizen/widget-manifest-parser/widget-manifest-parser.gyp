{
  'targets': [
    {
      'target_name': 'widget-manifest-parser',
      'type': 'shared_library',
      'dependencies': [
        'application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'sources': [
        'widget-manifest-parser.cc',
        'widget-manifest-parser.h',
      ],
      'include_dirs': [
        '..',
        '../..',
        '../../..',
      ],
    },
  ],
}
