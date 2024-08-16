# Codecks Bug & Feedback Reporter for Unreal Engine

> :warning: **Work in progress**: This plugin is still very much work in progress, currently there is very little documentation.

[`Documentation`](https://docs.google.com/document/d/114XjOAXXSG9q5k41aJJrlJY6LHzycnCGiXrVEVdEPT0).



Collect bugs and feedback right from your Unreal game and keep players in the loop what happens with their feedback.

This plugin is modified version of official [`codecks plugin for unreal engine`](https://github.com/codecks-io/codecks-unreal).
It was modified to suit our needs. 

## Features:
- Editor button that will teleport you to location encoded in json string that can be sent in report
- This button functionality can be modified or extended by inheriting json parser object
- Setting to set different url than official codecks server
- Fixed file upload on self-hosted servers
- Utils functions used in our reporting window (read/write to files, get game version)
- Send your reports to either bug report deck or feedback deck
- Change target API for internal and public testing
- Custom Log category for better readability



## License

The code is licensed under the MIT license. See [`LICENSE.md`](./LICENSE.md).
