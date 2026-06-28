import React from "react"
import withJuceComboBox, {WithJUCEComboBoxProps} from "./withJuceComboBox"
import "./styles/curveselector.scss"

interface TriggerButtonProps {
    parameterID: string
}

const CurveSelector: React.FunctionComponent<TriggerButtonProps & 
    WithJUCEComboBoxProps> = ({properties, value, onChange}) => {

    const curves = {
        exponential: <path d="M2 2 C6 24, 24 31, 52 31" stroke="currentColor" fill="none" strokeWidth="3"/>,
        linear: <path d="M2 2 L52 31" stroke="currentColor" fill="none" strokeWidth="3"/>,
        logarithmic: <path d="M2 2 C28 2, 44 10, 52 31" stroke="currentColor" fill="none" strokeWidth="3"/>,
    }

    const changeCurve = () => {
        let newIndex = value + 1
        if (newIndex > properties.choices.length - 1) newIndex = 0
        onChange(newIndex)
    }

    return (
        <div className="curve-selector" onClick={changeCurve}>
            <svg
                className="curve"
                viewBox="0 0 54 33"
                xmlns="http://www.w3.org/2000/svg">
                {Object.values(curves)[value]}
            </svg>
        </div>
    )
}

export default withJuceComboBox(CurveSelector)