#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "main.hpp"

std::string generated_out_file_name, generated_out_file_extension, generated_out_file_content,
  postfix;

int main(int argc, char *argv[])
{
  if (!verify_arguments(argc, argv)) return 1;

  for (int index = 2; index < argc - 1; index++)
  {
    std::string resource_path = argv[index];
    std::string resource_name, resource_extension;
    get_resource_file(resource_path, resource_name, resource_extension);

    if (!generate_resource(resource_path, resource_name, resource_extension)) return 1;
  }

  if (!write_to_output_file()) return 1;
}

bool verify_arguments(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: <postfix> <resource1> <resource2> ... <resourceN> <outfile>" << std::endl;
    return false;
  }

  generated_out_file_name = argv[argc - 1];
  size_t no_extension_start_pos = generated_out_file_name.find_last_of('/') == std::string::npos
                                    ? 0
                                    : generated_out_file_name.find_last_of('/') + 1;
  std::string generated_out_file_name_no_extension = generated_out_file_name.substr(
    no_extension_start_pos, generated_out_file_name.find_last_of('.') - no_extension_start_pos);
  generated_out_file_extension =
    generated_out_file_name.substr(generated_out_file_name.find_last_of('.') + 1);
  postfix = argv[1];

  if (generated_out_file_extension == "cpp")
    generated_out_file_content = "// This is an autogenerated file, do not edit.\n\n#include \"" +
                                 generated_out_file_name_no_extension + ".hpp\"\n";
  else if (generated_out_file_extension == "hpp")
    generated_out_file_content = "// This is an autogenerated file, do not edit.\n\n#pragma once\n";
  else
  {
    std::cout << "Invalid file extension: " << generated_out_file_extension << std::endl;
    return false;
  }

  return true;
}

std::string unsigned_char_to_hex(unsigned char ch)
{
  std::stringstream ss;
  ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
  return ss.str();
}

std::vector<unsigned char> read_text_file_as_binary(const std::string &filename)
{
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) { throw std::runtime_error("Could not open file"); }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<unsigned char> buffer((size_t)size);

  if (!file.read((char *)buffer.data(), size)) { throw std::runtime_error("Error reading file"); }

  return buffer;
}

void get_resource_file(const std::string &resource_path, std::string &resource_name,
                       std::string &resource_extension)
{
  size_t period_pos = resource_path.find_last_of('.');
  resource_name = resource_path.substr(0, period_pos);
  resource_extension = resource_path.substr(period_pos + 1);
}

void generate_cpp_png_resource(const std::string &resource_name, const int &width,
                               const int &height, const int &channels, unsigned char *data)
{
  std::string resource_out_file_content;
  resource_out_file_content +=
    "\nconst int " + resource_name + postfix + "_width" + " = " + std::to_string(width) + ";\n";
  resource_out_file_content +=
    "const int " + resource_name + postfix + "_height" + " = " + std::to_string(height) + ";\n";
  resource_out_file_content +=
    "const int " + resource_name + postfix + "_channels" + " = " + std::to_string(channels) + ";\n";
  resource_out_file_content +=
    "const unsigned char " + resource_name + postfix + "_data" + "[] = {\n";
  size_t data_size = size_t(width * height * channels);
  for (size_t i = 0; i < data_size; i++)
  {
    resource_out_file_content += unsigned_char_to_hex(data[i]) + ", ";
    if ((i + 1) % 16 == 0) { resource_out_file_content += "\n"; }
  }
  resource_out_file_content += "'\\0' };\n";
  generated_out_file_content += resource_out_file_content;
}

void generate_hpp_png_resource(const std::string &resource_name)
{
  std::string resource_out_file_content;
  resource_out_file_content += "\nextern const int " + resource_name + postfix + "_width;\n";
  resource_out_file_content += "extern const int " + resource_name + postfix + "_height;\n";
  resource_out_file_content += "extern const int " + resource_name + postfix + "_channels;\n";
  resource_out_file_content +=
    "extern const unsigned char " + resource_name + postfix + "_data[];\n";
  generated_out_file_content += resource_out_file_content;
}

bool generate_png_resource(const std::string &resource_path, std::string resource_name)
{
  int width, height, channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(resource_path.c_str(), &width, &height, &channels, 0);
  if (data == nullptr)
  {
    std::cout << "Failed to load image: " << resource_path << std::endl;
    return false;
  }

  resource_name = resource_name.substr(resource_name.find_last_of('/') + 1);
  if (generated_out_file_extension == "cpp")
    generate_cpp_png_resource(resource_name, width, height, channels, data);
  else
    generate_hpp_png_resource(resource_name);

  stbi_image_free(data);
  return true;
}

void generate_cpp_text_resource(const std::string &resource_path, const std::string &resource_name)
{
  std::string resource_out_file_content;
  std::vector<unsigned char> buffer = read_text_file_as_binary(resource_path);
  for (size_t i = 0; i < buffer.size(); i++)
  {
    resource_out_file_content += unsigned_char_to_hex(buffer[i]) + ", ";
    if ((i + 1) % 16 == 0) { resource_out_file_content += "\n"; }
  }
  generated_out_file_content += "\nconst char " + resource_name + postfix + "[] = {\n" +
                                resource_out_file_content + "'\\0' };\n";
}

void generate_hpp_text_resource(const std::string &resource_name)
{
  std::string resource_out_file_content;
  resource_out_file_content += "\nextern const char " + resource_name + postfix + "[];\n";
  generated_out_file_content += resource_out_file_content;
}

void generate_text_resource(const std::string &resource_path, std::string resource_name)
{
  std::string resource_out_file_content;
  resource_name = resource_name.substr(resource_name.find_last_of('/') + 1);
  if (generated_out_file_extension == "cpp")
    generate_cpp_text_resource(resource_path, resource_name);
  else
    generate_hpp_text_resource(resource_name);
}

bool generate_resource(const std::string &resource_path, const std::string &resource_name,
                       const std::string &resource_extension)
{
  if (resource_extension == "png")
  {
    if (!generate_png_resource(resource_path, resource_name)) return false;
  }
  else
    generate_text_resource(resource_path, resource_name);

  return true;
}

bool write_to_output_file()
{
  std::ofstream generated_out_file(generated_out_file_name);
  if (!generated_out_file.is_open())
  {
    std::cout << "Failed to open file: " << generated_out_file_name << std::endl;
    return false;
  }
  generated_out_file << generated_out_file_content;
  generated_out_file.close();

  return true;
}
