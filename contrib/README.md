# Contrib

## rpminspect-report

[rpminspect-report](https://softwarefactory-project.io/r/plugins/gitiles/software-factory/rpminspect-report)
is a web application dedicated to displaying rpminspect json reports. It provides a filtering
system to ease displaying alert messages of specific severities.

The application is deployed at https://fedora.softwarefactory-project.io/rpminspect-report
where a json report url can be provided.

To build your own instance of the web application or hack into the code then checkout the
repository.

```shell
git clone https://softwarefactory-project.io/r/software-factory/rpminspect-report
```

## viewer.html

This is a dynamic HTML viewer for JSON report files, similar in spirit to
rpminspect-report, but much more lightweight. It does not require any build
steps, and can just be copied as-is to any web server.

For testing and development, start a HTTP server on the contrib directory, for example:

    python3 -m http.server -d contrib

Then you can browse an existing result at `http://localhost:8000/results.html?url=...`, for example

http://localhost:8000/viewer.html?url=https://fedora.softwarefactory-project.io/logs//30/130/7237d3141f8ece8423b3345f17363df7b6711122/check/rpm-rpminspect/af18f07/result.json

Alternatively you can put any rpminspect result JSON file into `contrib/results.json` and open it at

http://localhost:8000/viewer.html?url=./results.json

## report-json2html.py

This is a json to html converter for rpminspect json reports.

To run it:

``` shell
python3 report-json2html.py report.json
```
