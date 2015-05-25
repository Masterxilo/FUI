using System.Threading;
using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
//using System.Speech.Recognition.SpeechRecognitionEngine;
using System.Speech.Recognition;
using System.Globalization;
using System.Windows.Forms;


using System.Text.RegularExpressions;

namespace FooBillardVoiceInput
{
      
    public class Program
    {
        static bool heardWord = false;
        static Dictionary<SpeechRecognitionEngine, ManualResetEvent> resets = new Dictionary<SpeechRecognitionEngine, ManualResetEvent>();
        static volatile String lastCommand = "";
        static volatile String lastSpokenPhrase = "";
        //static ManualResetEvent _completed = null;
        static String[] recognizedCommands;

        [DllImport("user32.dll")]
        static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);

        public static void Main(string[] args)
        {
            //only the first of these will run if they are not in seperate threads
            startRecogThread(true);
            //startRecogThread(false);
        }

        public static void startRecogThread(bool useLimitedDictionary)
        {
            //System.Collections.ObjectModel.ReadOnlyCollection<RecognizerInfo> recognizers=SpeechRecognitionEngine.InstalledRecognizers();
            //CultureInfo[] cultures=CultureInfo.GetCultures(CultureTypes.AllCultures);

            using (SpeechRecognitionEngine recog = new SpeechRecognitionEngine(new CultureInfo("en-US")))
            {
                resets.Add(recog,new ManualResetEvent(false));
                
                //_completed = new ManualResetEvent(false);

                GrammarBuilder gb = new GrammarBuilder();

                Choices commands = new Choices();
                recognizedCommands = new string[] { "shoot","hit","push", 
                                                    "stronger", "much stronger",
                                                    "weaker", "much weaker",
                                                   
                                                    "que",
                                                    "birdview",

                                                    "menu",
                                                    "select",
                                                    "up", 
                                                    "down",

                                                    "commands", "help", "what can I say",
                
                                                    "put here",
                                                    "revert","undo"};

                if (useLimitedDictionary)
                {
                    /////////////////
                    commands.Add(recognizedCommands);
                    gb.Append(commands);
                    /////////////////
                }

                else
                {
                    /////////////////
                    gb.AppendDictation();
                    gb.Culture = new CultureInfo("en-US");
                    /////////////////
                }
                

                // Create the Grammar instance.
                Grammar grammar = new Grammar(gb);

                recog.LoadGrammar(grammar);

                // Add a handler for the speech recognized event.
                if(useLimitedDictionary){
                    recog.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(FoundCommandWord);
                }
                else{
                    recog.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(FoundAnyWord);
                }
                //  sre.SpeechHypothesized += new EventHandler<SpeechRecognizedEventArgs>(speechRecognized);
                // Configure input to the speech recognizer.
                recog.SetInputToDefaultAudioDevice();

                // Start asynchronous, continuous speech recognition.
                recog.RecognizeAsync(RecognizeMode.Multiple);

                // Keep the console window open.
                Thread consoleThread = new Thread(keepConsoleOpen);
                consoleThread.IsBackground = false;//if all foreground threads are done the process terminates, backgroundT dont need to be terminated
                consoleThread.Start();

                //keepConsoleOpen();
                resets[recog].WaitOne();
                recog.Dispose();
            }

        }

        static void keepConsoleOpen()
        {
            while (true)
            {
               
                //Console.WriteLine("hey");
                Console.ReadLine();    
            }
        }


        public static String[] getRecongizedCommads()
        {
            return recognizedCommands;
        }

        public static bool wordRecognised() {
            return heardWord;
        }
        
        public static String getLastCommand(){
            String temp=lastCommand;
            lastCommand="";
            return temp;
        }

        public static String getLastSpokenPhrase()
        {
            String temp = lastSpokenPhrase;
            lastSpokenPhrase = "";
            return temp;
        }

        static void FoundAnyWord(object sender, SpeechRecognizedEventArgs e)
        {
            if (e.Result == null || e.Result.Confidence < 0.92)
                return;

            //if atleast one time the a word was recognized
            heardWord = true;

            //terminates speech recognition
            if (e.Result.Text.Equals("finish"))
            {
                Console.WriteLine("finished");
                resets[(SpeechRecognitionEngine)sender].Set();
                //_completed.Set();
                return;
            }

            //if (!recognizedCommands.Contains(e.Result.Text))
            //{
                //Console.WriteLine("Unrecognized words: " + e.Result.Text);
                //remember last valid command
                lastSpokenPhrase = e.Result.Text;
            //}
        }

        static void FoundCommandWord(object sender, SpeechRecognizedEventArgs e)
        {
            if (e.Result == null || e.Result.Confidence<0.92) 
                return;

            //if atleast one time the a word was recognized
            heardWord = true;

            //terminates speech recognition
            if (e.Result.Text.Equals("finish"))
            {
                Console.WriteLine("finished");
                resets[(SpeechRecognitionEngine)sender].Set();
                //_completed.Set();
                return;
            }

            //if (recognizedCommands.Contains(e.Result.Text))
            //{
                //Console.WriteLine("Recognized words: " + e.Result.Text);
                //remember last valid command
                lastCommand = e.Result.Text;
            //}
        }


        /*static void SpeechRecognizedHandler(object sender, SpeechRecognizedEventArgs e)
        {
            if (e.Result == null) return;

            // Add event handler code here.

            // The following code illustrates some of the information available
            // in the recognition result.
            Console.WriteLine("Recognition result summary:");
            Console.WriteLine(
              "  Recognized phrase: {0}\n" +
              "  Confidence score {1}\n" +
              "  Grammar used: {2}\n",
              e.Result.Text, e.Result.Confidence, e.Result.Grammar.Name);

            // Display the semantic values in the recognition result.
            Console.WriteLine("  Semantic results:");
            foreach (KeyValuePair<String, SemanticValue> child in e.Result.Semantics)
            {
                Console.WriteLine("    The {0} city is {1}",
                  child.Key, child.Value.Value ?? "null");
            }
            Console.WriteLine();

            // Display information about the words in the recognition result.
            Console.WriteLine("  Word summary: ");
            foreach (RecognizedWordUnit word in e.Result.Words)
            {
                Console.WriteLine(
                  "    Lexical form ({1})" +
                  " Pronunciation ({0})" +
                  " Display form ({2})",
                  word.Pronunciation, word.LexicalForm, word.DisplayAttributes);
            }

            // Display information about the audio in the recognition result.
            Console.WriteLine("  Input audio summary:\n" +
              "    Candidate Phrase at:       {0} mSec\n" +
              "    Phrase Length:             {1} mSec\n" +
              "    Input State Time:          {2}\n" +
              "    Input Format:              {3}\n",
              e.Result.Audio.AudioPosition,
              e.Result.Audio.Duration,
              e.Result.Audio.StartTime,
              e.Result.Audio.Format.EncodingFormat);

            // Display information about the alternate recognitions in the recognition result.
            Console.WriteLine("  Alternate phrase collection:");
            foreach (RecognizedPhrase phrase in e.Result.Alternates)
            {
                Console.WriteLine("    Phrase: " + phrase.Text);
                Console.WriteLine("    Confidence score: " + phrase.Confidence);
            }
        }
        */


        private const int APPCOMMAND_VOLUME_MUTE = 0x80000;
        private const int APPCOMMAND_VOLUME_UP = 0xA0000;
        private const int APPCOMMAND_VOLUME_DOWN = 0x90000;
        private const int WM_APPCOMMAND = 0x319;

        [DllImport("user32.dll")]
        public static extern IntPtr SendMessageW(IntPtr hWnd, int Msg,
            IntPtr wParam, IntPtr lParam);
    }
}
