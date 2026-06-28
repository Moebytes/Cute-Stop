import React from "react"
import withJuceToggleButton, {WithJUCEToggleButtonProps} from "./withJuceToggleButton"
import StopIcon from "../assets/stop.svg"
import StartIcon from "../assets/start.svg"
import "./styles/triggerbutton.scss"

interface TriggerButtonProps {
    parameterID: string
}

const TriggerButton: React.FunctionComponent<TriggerButtonProps & 
    WithJUCEToggleButtonProps> = ({value, onChange}) => {
    return (
        value ?
        <StartIcon className="trigger-button" onClick={() => onChange(!value)}/> :
        <StopIcon className="trigger-button" onClick={() => onChange(!value)}/>
    )
}

export default withJuceToggleButton(TriggerButton)