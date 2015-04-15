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

    class Program
    {

        [DllImport("user32.dll")]
        static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);

        static void Main(string[] args)
        {
            //System.Collections.ObjectModel.ReadOnlyCollection<RecognizerInfo> recognizers=SpeechRecognitionEngine.InstalledRecognizers();
            //CultureInfo[] cultures=CultureInfo.GetCultures(CultureTypes.AllCultures);

            using (SpeechRecognitionEngine recog = new SpeechRecognitionEngine(new CultureInfo("en-US")))
            {
                GrammarBuilder builder = new GrammarBuilder();
                builder.AppendDictation();
                builder.Culture = new CultureInfo("en-US");
                Grammar grammar = new Grammar(builder);

                recog.LoadGrammar(grammar);

                // Add a handler for the speech recognized event.
                recog.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(speechRecognized);
                //  sre.SpeechHypothesized += new EventHandler<SpeechRecognizedEventArgs>(speechRecognized);
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

        static void cmd(String cmd)
        {
            string strCmdText;
            strCmdText = "/C " + cmd;
            System.Diagnostics.Process.Start("CMD.exe", strCmdText);
        }

        private static string GetActiveWindowTitle()
        {
            const int nChars = 256;
            StringBuilder Buff = new StringBuilder(nChars);
            IntPtr handle = GetForegroundWindow();

            if (GetWindowText(handle, Buff, nChars) > 0)
            {
                return Buff.ToString();
            }
            return null;
        }

        static void SpeechRecognizedHandler(object sender, SpeechRecognizedEventArgs e)
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

        private const int APPCOMMAND_VOLUME_MUTE = 0x80000;
        private const int APPCOMMAND_VOLUME_UP = 0xA0000;
        private const int APPCOMMAND_VOLUME_DOWN = 0x90000;
        private const int WM_APPCOMMAND = 0x319;

        [DllImport("user32.dll")]
        public static extern IntPtr SendMessageW(IntPtr hWnd, int Msg,
            IntPtr wParam, IntPtr lParam);

        private static void btnMute_Click()
        {
            SendMessageW(GetForegroundWindow(), WM_APPCOMMAND, GetForegroundWindow(),
                  (IntPtr)APPCOMMAND_VOLUME_MUTE);
        }

        private static void btnDecVol_Click()
        {
            SendMessageW(GetForegroundWindow(), WM_APPCOMMAND, GetForegroundWindow(),
                  (IntPtr)APPCOMMAND_VOLUME_DOWN);
        }

        private static void btnIncVol_Click()
        {
            SendMessageW(GetForegroundWindow(), WM_APPCOMMAND, GetForegroundWindow(),
                  (IntPtr)APPCOMMAND_VOLUME_UP);
        }

        // Handle the SpeechRecognized event.
        static void speechRecognized(object sender, SpeechRecognizedEventArgs e)
        {

            SpeechRecognizedHandler(sender, e);
            string t = e.Result.Text;
            if (doit(t)) return;
            foreach (RecognizedPhrase phrase in e.Result.Alternates)
            {
                if (doit(phrase.Text)) return;
            }
        }
        static int ss = 10; // volume steps /2
        static bool doit(string t)
        {
            //Console.WriteLine(GetActiveWindowTitle());
            if (t == "start chrome" || t == "start from") { cmd("ch.bat"); return true; }
            if (t == "decrease volume" || t == "reduce volume" || t == "reduce the volume" || t == "turn down the volume" || t == "reduced volume" || t == "not so loud" || t == "degrees volume"
              || t == "decreasing volume" || t == "reducing volume"
              || t == "make more slient"
              || t == "decrease the volume" || t == "decreased the volume"
              || t == "decrease a volume"
              )
            {
                for (int i = 0; i < ss; i++) btnDecVol_Click();
                return true;
            }


            if (t == "increase volume" || t == "increase volume" || t == "increase in volume" || t == "increased volume" || t == "make much louder" || t == "louder" || t == "turn up the volume" || t == "make louder"
              || t == "increases volume"
              || t == "increase the volume"
              )
            {
                for (int i = 0; i < ss; i++) btnIncVol_Click();
                return true;
            }

            if (t == "turn sound off" || t == "turn off sound" || t == "sound off" || t == "turn sound on" || t == "turn on sound" || t == "sound on" || t == "sound of" || t == "disable sound" || t == "enable sound" || t == "enables sound" || t == "shut up" || t == "shutup" || t == "toggle sound" || t == "no sound" || t == "unmute" || t == "mute" | t == "enable some")
            {
                btnMute_Click();
                return true;
            }
            if (t == "open new tab"
              || t == "open you tab" || t == "open you to" || t == "open your tab"
              || t == "start you tab"
              || t == "start new tab" || t == "create new tab" || t == "create a new tab"
              || t == "you tab"
              || t == "new tab"

              || t == "open new tap" || t == "open a new tap"
              || t == "open you tap"
              || t == "start you tap"
              || t == "start new tap"
              || t == "you tap"
              || t == "new tap"
              )
            {
                SendKeys.SendWait("^t"); return true;
            }

            if (t == "go back" || t == "go back one page" || t == "go one page back"
              )
            {
                SendKeys.SendWait("%{LEFT}"); return true;
            } if (t == "go forward" || t == "go forward one page" || t == "go one page forward" || t == "go forward a page"
              || t == "go forward again"
              )
            {
                SendKeys.SendWait("%{LEFT}"); return true;
            }

            if (t == "close tab" || t == "close this tab" ||
              t == "close gap" || t == "close this gap" || t == "quit this tab"
              || t == "close current tab" || t == "close the current tab"
             )
            {
                SendKeys.SendWait("^w"); return true;
            }
            if (t == "show history"
             )
            {
                SendKeys.SendWait("^h"); //SendKeys.SendWait("%c");
                SendKeys.SendWait("^y"); return true;
            }

            string pattern = @"^search (.*?)(| in (|a|the) new (to|tab|ten|gap))$";
            Match match = Regex.Match(t, pattern);
            foreach (Group g in match.Groups)
                Console.WriteLine("|" + g.Value);
            if (match.Success)
            {
                if (match.Groups[2].Value.Length > 0)
                {
                    SendKeys.SendWait("^t"); Thread.Sleep(500);

                }
                else
                    SendKeys.SendWait("^l");
                SendKeys.SendWait(match.Groups[1].Value);
                SendKeys.SendWait("{ENTER}"); return true;
            }


            return false;
        }
    }
}
