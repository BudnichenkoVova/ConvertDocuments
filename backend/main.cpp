#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <iostream>
#include <cstdlib>

using namespace web::http::experimental::listener;
using namespace web;
using namespace http;

int main() {
    http_listener listener(L"http://localhost:8080");

    // Temporary folder to store uploaded files
    const std::wstring tempFolder = L"./uploads";

    listener.support(methods::POST, [tempFolder](http_request request) {
        // Check the request URI to determine the conversion type
        utility::string_t endpoint = request.relative_uri().path();
        auto response = std::make_shared<http_response>(status_codes::OK);

        if (endpoint == L"/wordToExcel") {
            // Handle Word to Excel conversion
            utility::string_t tempFileName = tempFolder + L"/uploaded_file.docx"; // Adjust file name and extension

            // Define the Pandoc conversion command for Word to Excel
            std::string conversionCommand = "pandoc " +
                utility::conversions::to_utf8string(tempFileName) + " " +
                "--to=xlsx " +          // Output format is Excel
                "--output " + utility::conversions::to_utf8string(tempFolder + L"/converted_file.xlsx");

            // Execute the Pandoc conversion command
            int conversionResult = std::system(conversionCommand.c_str());

            if (conversionResult == 0) {
                // Conversion was successful, send the converted file as a response
                concurrency::streams::istream convertedFileStream = file_stream<uint8_t>::open_istream(tempFolder + L"/converted_file.xlsx");
                response->headers().set_content_type(U("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
                response->set_body(convertedFileStream);
            } else {
                // Conversion failed, send an error response
                response->set_status_code(status_codes::InternalError);
                response->headers().set_content_type(U("application/json"));
                json::value jsonResponse;
                jsonResponse[U("error")] = json::value::string(U("Conversion failed"));
                response->set_body(jsonResponse);
            }

        } else if (endpoint == L"/excelToWord") {
            // Handle Excel to Word conversion
            utility::string_t tempFileName = tempFolder + L"/uploaded_file.xlsx"; // Adjust file name and extension

            // Define the Pandoc conversion command for Excel to Word
            std::string conversionCommand = "pandoc " +
                utility::conversions::to_utf8string(tempFileName) + " " +
                "--to=docx " +          // Output format is Word
                "--output " + utility::conversions::to_utf8string(tempFolder + L"/converted_file.docx");

            // Execute the Pandoc conversion command
            int conversionResult = std::system(conversionCommand.c_str());

            if (conversionResult == 0) {
                // Conversion was successful, send the converted file as a response
                concurrency::streams::istream convertedFileStream = file_stream<uint8_t>::open_istream(tempFolder + L"/converted_file.docx");
                response->headers().set_content_type(U("application/msword"));
                response->set_body(convertedFileStream);
            } else {
                // Conversion failed, send an error response
                response->set_status_code(status_codes::InternalError);
                response->headers()->set_content_type(U("application/json"));
                json::value jsonResponse;
                jsonResponse[U("error")] = json::value::string(U("Conversion failed"));
                response->set_body(jsonResponse);
            }

        } else {
            // Handle other endpoints or return an error
            request.reply(status_codes::NotFound, U("Endpoint not found"));
            return;
        }

        // Reply with the response
        request.reply(*response);
    });

    listener.open().wait();

    std::wcout << L"Listening on: " << listener.uri().to_string() << std::endl;

    while (true);

    return 0;
}
