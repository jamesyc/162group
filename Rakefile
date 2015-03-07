# coding: utf-8

# Configuration settings

# These variables should be updated every time a checkpoint deadline is
# reached, or a new project is released.

proj = "proj1"
checkpoint = "checkpoint3"
proj_dir = "pintos/src"

ag_branch = "ag"
release_branch = "release"



# Rake tasks - don't modify anything below this line

module Colors
  def colorize(text, color_code)
    "\033[#{color_code}m#{text}\033[0m"
  end

  {
    :black    => 30,
    :red      => 31,
    :green    => 32,
    :yellow   => 33,
    :blue     => 34,
    :magenta  => 35,
    :cyan     => 36,
    :white    => 37
  }.each do |key, color_code|
    define_method key do |text|
      colorize(text, color_code)
    end
  end
end

module Helpers
  def output(success)
    return ": " + (success ? green("SUCCESS") : red("FAILURE"))
  end
end

include Colors
include Helpers

desc "Submit assignment to branch"
task :push, [:branch, :proj, :checkpoint, :proj_dir] do |t, args|
    full_branch = "#{args.branch}/#{args.proj}/#{args.checkpoint}"

    clean = system("make clean -C #{args.proj_dir}")
    new_branch = system("git branch #{full_branch}")
    checkout = system("git checkout #{full_branch}")
    merge = system("git merge master")
    push = system("git push -f group #{full_branch}")
    master = system("git checkout master")

    puts "Clean executables" + output(clean)
    puts "Switch to #{full_branch}" + output(checkout)
    puts "Merge files from master into #{full_branch}" + output(merge)
    puts "Push to #{full_branch} on group remote" + output(push)
    puts "Switch back to master" + output(master)
end

desc "Submit assignment to release"
task :release, [:proj, :checkpoint, :proj_dir] do |t, args|
    Rake::Task[:push].invoke("#{release_branch}", args.proj, args.checkpoint, args.proj_dir)
    puts cyan "Submitted the project! Sit back and relax."
end

desc "Submit assignment to autograder"
task :ag, [:proj, :checkpoint, :proj_dir] do |t, args|
    Rake::Task[:default].invoke("#{ag_branch}", args.proj, args.checkpoint, args.proj_dir)
    puts magenta "Don't forget to push to release! #{red("(rake release)")}"
end

desc "Default task"
task :default, [:branch, :proj, :checkpoint, :proj_dir] do |t, args|
    args.with_defaults(branch: "#{ag_branch}")
    args.with_defaults(proj: "#{proj}")
    args.with_defaults(checkpoint: "#{checkpoint}")
    args.with_defaults(proj_dir: "#{proj_dir}")

    Rake::Task[:push].invoke(args.branch, args.proj, args.checkpoint, args.proj_dir)
end


