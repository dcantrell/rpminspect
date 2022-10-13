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

## report-json2html.py

This is a json to html converter for rpminspect json reports.

To run it:

``` shell
python3 report-json2html.py report.json
```
