
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
//using System.Speech.Recognition.SpeechRecognitionEngine;
using System.Speech.Recognition;
using System.Globalization;
using System.Windows.Forms;


namespace FooBillardVoiceInput
{
    
    class Program
    {

        static void Main(string[] args){
            //System.Collections.ObjectModel.ReadOnlyCollection<RecognizerInfo> recognizers=SpeechRecognitionEngine.InstalledRecognizers();
            //CultureInfo[] cultures=CultureInfo.GetCultures(CultureTypes.AllCultures);
           
            using (SpeechRecognitionEngine recog = new SpeechRecognitionEngine(new CultureInfo("en-US")))
            {
                // Create and load a grammar.
                Choices voiceCommands = new Choices(new string[] { "green", "red"});
                recog.LoadGrammar(new Grammar(new GrammarBuilder(voiceCommands)));

                // Add a handler for the speech recognized event.
                recog.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(speechRecognized);

                // Configure input to the speech recognizer.
                recog.SetInputToDefaultAudioDevice();

                // Start asynchronous, continuous speech recognition.
                recog.RecognizeAsync(RecognizeMode.Multiple);

                // Keep the console window open.
                while (true)
                {
                    Console.ReadLine();
                }
            }
        }

        // Handle the SpeechRecognized event.
        static void speechRecognized(object sender, SpeechRecognizedEventArgs e)
        {
            Console.WriteLine("Recognized text: " + e.Result.Text+" | Time: "+DateTime.Now.TimeOfDay.TotalSeconds);
            if (e.Result.Text == "green")
            {
                SendKeys.SendWait("green");
            }
            else if (e.Result.Text == "red")
            {
                SendKeys.SendWait("red");
            }
        }
    }
}
