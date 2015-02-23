# coding: utf-8

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


proj = "proj1"
proj_dir = "pintos/src"
checkpoint = "checkpoint2"

ag_branch = "ag"
release_branch = "release"

desc "Submit assignment to branch"
task :push, [:branch, :proj] do |t, args|
    args.with_defaults(branch: "#{ag_branch}")
    args.with_defaults(proj: "#{proj}")

    branch = args.branch
    proj = args.proj

    full_branch = "#{branch}/#{proj}/#{checkpoint}"

    clean = system("make clean -C #{proj_dir}")
    new_branch = system("git branch #{full_branch}")
    checkout = system("git checkout #{full_branch}")
    merge = system("git merge master")
    push = system("git push -f personal #{full_branch}")
    master = system("git checkout master")

    puts "Clean executables" + output(clean)
    puts "Switch to #{full_branch}" + output(checkout)
    puts "Merge files from master into #{full_branch}" + output(merge)
    puts "Push to #{full_branch} on remote" + output(push)
    puts "Switch back to master" + output(master)
end

desc "Submit assignment to release"
task :release, [:proj] do |t, args|
    Rake::Task[:push].invoke("#{release_branch}", args.proj)
end

desc "Submit assignment to autograder"
task :ag, [:proj] do |t, args|
    Rake::Task[:push].invoke("#{ag_branch}", args.proj)
    puts magenta "Don't forget to push to #{red("release")}!"
end

desc "Default task"
task :default => [:ag] do
end


