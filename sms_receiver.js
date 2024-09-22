const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const axios = require("axios");

// List available serial ports
SerialPort.list()
  .then((ports) => {
    console.log("Available Ports:");
    ports.forEach((port) => {
      console.log(`- ${port.path}`);
    });

    // Check if COM5 exists
    const targetPort = "COM5";
    const isPortAvailable = ports.some((port) => port.path === targetPort);
    if (!isPortAvailable) {
      console.error(
        `Error: Port ${targetPort} is not available. Please check the connected devices.`
      );
      return;
    }

    // Open the serial port (ensure the port 'COM5' matches your Arduino's connection)
    const port = new SerialPort({ path: targetPort, baudRate: 9600 });

    // Use the ReadlineParser to handle incoming data line by line
    const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));

    // Define the API endpoint where the SMS data will be sent
    const apiUrl = "http://localhost:3000/api/sms";

    let isNextLineSMSContent = false;

    parser.on("data", async (data) => {
      data = data.trim(); // Clean up the data
      console.log(`Received data: ${data}`);

      // If the previous line was "+CMT:", this line should be the SMS content
      if (isNextLineSMSContent) {
        isNextLineSMSContent = false;

        if (data.length > 0) {
          console.log(`Processing SMS content: ${data}`);

          // Prepare the payload to send to the API
          const jsonPayload = {
            house_no: data, // Assuming the SMS content is the house number
          };
          console.log({ jsonPayload });
          try {
            // Send the payload to the API
            const response = await axios.post(apiUrl, jsonPayload);
            console.log(`API Response: ${response.status} - ${response.data}`);
          } catch (error) {
            console.error(`API request failed: ${error.message}`);
          }
        } else {
          console.log("Received invalid or empty SMS content, ignoring...");
        }
      } else if (data.startsWith("+CMT:")) {
        // The next line should contain the SMS content
        isNextLineSMSContent = true;
        console.log("SMS header detected, awaiting SMS content...");
      } else {
        console.log("Received non-SMS data, ignoring...");
      }
    });

    // Handle serial port errors
    port.on("error", (err) => {
      console.error(`Serial Port Error: ${err.message}`);
    });

    port.on("open", () => {
      console.log("Serial port opened successfully.");
    });
  })
  .catch((err) => {
    console.error("Error listing serial ports:", err);
  });
