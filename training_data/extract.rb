#!/usr/bin/env ruby

if ARGV.size < 2
	puts "Missing input directory."
	exit
end
if ARGV.size < 3
	puts "Missing output directory."
	exit
end
if ARGV.size < 4
	puts "Missing at least one class."
	exit
end
input_dir = ARGV[0]
output_dir = ARGV[1]
classes = []
for i in (2..ARGV.size - 1)
	classes.push ARGV[i]
end

classes.each do |c|
	for i in (1..20)
		input_file = "#{input_dir}\\#{c}#{i < 10? "0" : ""}#{i}.wav"
		puts input_file
		output_file = "#{output_dir}\\#{c}#{i < 10? "0" : ""}#{i}.txt"
		system("asr_take01.exe -e #{input_file} > #{output_file}")
	end
end