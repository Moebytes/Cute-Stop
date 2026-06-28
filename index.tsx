import React, {useState, useEffect} from "react"
import {createRoot} from "react-dom/client"
import TriggerButton from "./components/TriggerButton"
import CurveSelector from "./components/CurveSelector"
import Knob from "./components/Knob"
import PresetBar from "./components/PresetBar"
import DarkIcon from "./assets/dark.svg"
import LightIcon from "./assets/light.svg"
import parameters from "./processor/parameters.json"
import functions from "./structures/Functions"
import "./index.scss"

const darkColorList = {
    "--background": "#220E1A",
    "--textColor": "#FFFFFF"
}

const lightColorList = {
    "--background": "#FFBEE6",
    "--textColor": "#000000",
}

type ThemeContextType = {theme: string; setTheme: React.Dispatch<React.SetStateAction<string>>}
export const ThemeContext = React.createContext<ThemeContextType>({theme: "", setTheme: () => null})

const App: React.FunctionComponent = () => {
    const [theme, setTheme] = useState(localStorage.getItem("theme") || "light")
    
    useEffect(() => {
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [])

    useEffect(() => {
        const colorList = theme === "light" ? lightColorList : darkColorList
        for (const [key, color] of Object.entries(colorList)) {
            document.documentElement.style.setProperty(key, color)
        }
        localStorage.setItem("theme", theme)
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [theme])

    const toggleTheme = () => {
        setTheme((prev) => prev === "light" ? "dark" : "light")
    }

    const filter = functions.calculateFilter("#FF4EA7")

    return (
        <div className="app">
            <ThemeContext.Provider value={{theme, setTheme}}>
            <div className="title-container">
                <span className="title-text">Cute Stop</span>
                {theme === "light" ? 
                <DarkIcon className="theme-icon" style={{filter}} onClick={toggleTheme}/> :
                <LightIcon className="theme-icon" style={{filter}} onClick={toggleTheme}/>}
            </div>
            <div className="trigger-container">
                <TriggerButton parameterID={parameters.trigger.id}/>
            </div>
            <div className="knobs-container">
                <Knob 
                    label={"STOP"} 
                    parameterID={parameters.stopTime.id}
                    color="#FF4EA7"/>
                <CurveSelector 
                    parameterID={parameters.curve.id}/>
                <Knob 
                    label={"START"} 
                    parameterID={parameters.startTime.id}
                    color="#FF4EA7"/>
            </div>
            <div className="preset-container">
                <PresetBar/>
            </div>
            </ThemeContext.Provider>
        </div>
    )

}

const root = createRoot(document.getElementById("root")!)
root.render(<App/>)