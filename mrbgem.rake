MRuby::Gem::Specification.new('mruby-cloudwalk-platform') do |spec|
  spec.license = 'MIT'
  spec.authors = 'CloudWalk Inc.'

  AVIXY_SDK_WORKSPACE = "/home/avixy/workspace"
  AVIXY_LIBRARIES_PATH = "#{AVIXY_SDK_WORKSPACE}/libraries"

  spec.cc.include_paths << "#{build.root}/src"
  spec.cc.include_paths << "#{AVIXY_LIBRARIES_PATH}/core/inc" 
  spec.cc.include_paths << "#{AVIXY_LIBRARIES_PATH}/avixy/inc" 
  spec.cc.include_paths << "#{AVIXY_LIBRARIES_PATH}/magcard/inc"
  spec.cc.include_paths << "#{AVIXY_SDK_WORKSPACE}/avixy/avx3400/include"

  # set the Avixy library path and libraries
  if spec.cc.flags.grep(/AVX_MODEL/).size > 0
    spec.linker.library_paths << "#{AVIXY_SDK_WORKSPACE}/libraries/avixy/SharedLib"    
    spec.linker.libraries << 'avixy'  
  end

  # Add compile flags
  # spec.cc.flags << ''

  # Add cflags to all
  # spec.mruby.cc.flags << '-g'

  # Add libraries
  # spec.linker.libraries << 'external_lib'

  # Default build files
  spec.rbfiles = Dir.glob("#{dir}/mrblib/*.rb")
  # spec.objs = Dir.glob("#{dir}/src/*.{c,cpp,m,asm,S}").map { |f| objfile(f.relative_path_from(dir).pathmap("#{build_dir}/%X")) }
  # spec.test_rbfiles = Dir.glob("#{dir}/test/*.rb")
  # spec.test_objs = Dir.glob("#{dir}/test/*.{c,cpp,m,asm,S}").map { |f| objfile(f.relative_path_from(dir).pathmap("#{build_dir}/%X")) }
  # spec.test_preload = 'test/assert.rb'

  # Values accessible as TEST_ARGS inside test scripts
  # spec.test_args = {'tmp_dir' => Dir::tmpdir}
  #spec.add_dependency 'mruby-platform-network'
  spec.add_dependency 'mruby-io'
  spec.add_dependency 'mruby-time'
end
