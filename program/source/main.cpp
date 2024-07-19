#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string char_to_hex(unsigned char ch)
{
  std::stringstream ss;
  ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
  return ss.str();
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: <resource1> <resource2> ... <resourceN> <postfix> <outfile>" << std::endl;
    return 1;
  }

  std::string generated_file_name = argv[argc - 1];
  std::string generated_file_content =
    "// This is an autogenerated file, do not edit.\n\n#pragma once\n\n";

  std::string postfix = argv[argc - 2];
  for (int index = 1; index < argc - 2; index++)
  {
    std::string resource_path = argv[index];
    size_t period_pos = resource_path.find_last_of('.');
    std::string resource_name = resource_path.substr(0, period_pos);
    std::string resource_extension = resource_path.substr(period_pos + 1);

    std::string resource_content;
    std::ifstream resource_file(resource_path);
    if (!resource_file.is_open())
    {
      std::cout << "Failed to open file: " << resource_path << std::endl;
      return 1;
    }

    if (resource_extension == "png")
    {
      int width, height, channels;
      unsigned char *data = stbi_load(resource_path.c_str(), &width, &height, &channels, 0);
      if (data == nullptr)
      {
        std::cout << "Failed to load image: " << resource_path << std::endl;
        return 1;
      }

      resource_name = resource_name.substr(resource_name.find_last_of('/') + 1);
      resource_content += "constexpr int " + resource_name + postfix + "_width" + " = " +
                          std::to_string(width) + ";\n";
      resource_content += "constexpr int " + resource_name + postfix + "_height" + " = " +
                          std::to_string(height) + ";\n";
      resource_content += "constexpr int " + resource_name + postfix + "_channels" + " = " +
                          std::to_string(channels) + ";\n";
      resource_content +=
        "constexpr unsigned char " + resource_name + postfix + "_data" + "[] = {\n";
      size_t data_size = size_t(width * height * channels);
      for (size_t i = 0; i < data_size; i++)
      {
        resource_content += char_to_hex(data[i]) + ", ";
        if ((i + 1) % 16 == 0) { resource_content += "\n"; }
      }
      resource_content += "};\n";
      generated_file_content += resource_content;

      stbi_image_free(data);
    }
    else
    {
      std::string line;
      while (std::getline(resource_file, line)) { resource_content += line + "\n"; }
      resource_name = resource_name.substr(resource_name.find_last_of('/') + 1);
      generated_file_content +=
        "constexpr char " + resource_name + postfix + "[] = R\"(\n" + resource_content + ")\";\n";
    }
    resource_file.close();
  }

  std::ofstream generated_file(generated_file_name);
  if (!generated_file.is_open())
  {
    std::cout << "Failed to open file: " << generated_file_name << std::endl;
    return 1;
  }
  generated_file << generated_file_content;
  generated_file.close();

  return 0;
}
